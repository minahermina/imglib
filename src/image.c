/*
A minimal Image Processing in pure C library
Copyright (C) 2025  Mina Albert Saeed <mina.albert.saeed@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>

#include "image.h"

/* Macros */
#define MAXLINE 1024
#define CHUNK_SIZE 8192
#define MAX(A, B)                 ((A) > (B) ? (A) : (B))
#define MIN(A, B)                 ((A) < (B) ? (A) : (B))
#define FLOOR(x)                  ((int)(x) - ((x) < 0 && (x) != (int)(x)))
#define ABS(x)                    ((x) < 0 ? -(x) : (x))
#define P(x)                      (x <= 0 ? 0 : x)
#define IMG_PIXEL_PTR(img, x, y)  ((uint8_t*)((img)->data + (y) * (img)->stride + (x) * (img)->channels))
#define IMG_ARR_SIZE(x)           (sizeof(x) / sizeof((x)[0]))
#define VAR(var)                  fprintf(stderr, "[DEBUG] %s = %d\n", #var, (var))
#define KERNEL_ASSERT(x)          assert((x).data != NULL)

/*TODO: find more flexible & dynamic way for this (more than 2 bytes))*/
#define HEX_TO_ASCII(hex)               (char[]){(char)((hex) >> 8), (char)((hex) & 0xFF), '\0'}

/* TODO: Replace error-handling macros with a more robust error-handling mechanism */
#define CHECK_ALLOC(ptr) \
    CHECK_COND(ptr == NULL, "Memmory Allocation failed", NULL)\

#define CHECK_PTR(ptr, ret_val) \
    CHECK_COND(ptr == NULL, " ", ret_val) \

#define CHECK_COND(ex, msg, ret_val) \
    if (ex) { \
        fprintf(stderr, "Error: (%s) is True!\n(function: %s, line %d, file %s)\n", #ex, __func__, __LINE__, __FILE__); \
        return ret_val; \
    }

static inline uint32_t
calc_stride(uint16_t width, uint8_t channels)
{
    return (((uint32_t) width * (uint32_t)channels + 15) & ~(uint32_t)15);
}

/* TODO: Replace current_pos logic with x, y to use img_getpx & img_setpx*/
static int8_t
addpixel(ImagePtr img, const uint8_t *pixel, uint32_t *current_pos)
{
    uint8_t *p, i;
    uint32_t x, y;

    CHECK_COND(img == NULL || pixel == NULL || current_pos == NULL,
               "Error: Invalid input (img, pixel, or position is NULL).\n",
               -1);

    if (*current_pos >= (uint32_t)(img->width * img->height)) {
        fprintf(stderr, "Error: Exceeded image capacity.\n");
        return -1;
    }

    x = *current_pos % img->width;
    y = *current_pos / img->width;
    p = IMG_PIXEL_PTR(img, x, y);

    for (i = 0; i < img->channels; i++) {
        p[i] = pixel[i];
    }

    (*current_pos)++;

    return 1;
}

ImagePtr
img_create(uint16_t width, uint16_t height, uint8_t channels)
{
    ImagePtr img;

    /*TODO: Extend channel support beyond the current 1-4 limit to accommodate additional color spaces:
     * - LAB (3 channels)
     * - CMYK (4 channels)
     * - CMYKA (5 channels)
     * - etc.
     */
    CHECK_COND(width == 0 || height == 0 || channels < 1 || channels > 4,
           "Invalid image dimensions or channels (width/height must be > 0, channels must be 1-4)",
           NULL);

    img = (ImagePtr) malloc(sizeof(Image));
    CHECK_ALLOC(img)

    img->stride = calc_stride(width, channels);
    img->data = (uint8_t*) malloc(height * img->stride);
    CHECK_PTR(img->data, NULL);

    memset(img->data, 0, height * img->stride);

    img->width = width;
    img->height = height;
    img->channels = channels;
    img->type = -1;

    return img;
}

ImagePtr
img_load(const char* file)
{
    ImagePtr img = NULL;
    ImgType type;

    CHECK_COND(file == NULL , "" ,NULL );

    type = img_type(file);
    switch(type) {
        case IMG_PPM_BIN: /* FALLTHROUGH */
        case IMG_PPM_ASCII:
        case IMG_PGM_BIN:
        case IMG_PGM_ASCII:
            img = img_loadpnm(file, type);
            if (img) {
                img->type = type;
            }else{
                fprintf(stderr, "Error loading image: %s\n", file);
                return NULL;
            }
            break;
        default:
            fprintf(stderr, "Unsupported or invalid image format\n");
    }

    return img;
}


ImgType
img_type(const char *file)
{
    FILE* f;
    char magic[3] = {0};
    char line[MAXLINE];

    f = fopen(file, "rb");
    CHECK_PTR(f, IMG_UNKNOWN);

    while (fgets(line, MAXLINE, f) != NULL) {
        if (line[0] == '#') {
            continue;  // Skip comment lines
        }

        if (sscanf(line, "%2s", magic) == 1) {
            break;
        }
    }

    uint16_t magic_ = (magic[0] << 8) | magic[1];

    fclose(f);

    switch(magic_) {
        case IMG_PPM_BIN:
            return IMG_PPM_BIN;
        case IMG_PPM_ASCII:
            return IMG_PPM_ASCII;
        case IMG_PGM_BIN:
            return IMG_PGM_BIN;
        case IMG_PGM_ASCII:
            return IMG_PGM_ASCII;
        default:
            return IMG_UNKNOWN;
    }
}



/* TODO: Optimize this funciton*/
ImagePtr
img_loadpnm(const char* file, ImgType type)
{
    ImagePtr img;
    ssize_t imgfile;
    FILE* tmp_file;
    uint8_t pixel[3], channels, ch, leftover[2] = {0}, leftover_size = 0;
    uint16_t w, h;
    uint32_t i, curr_pos = 0, bytesread = 0;
    char line[MAXLINE], chunk[CHUNK_SIZE];

    tmp_file = fopen(file, "rb");
    if (tmp_file == NULL)
        return NULL;

    switch(type) {
        case IMG_PPM_BIN: /* FALLTHROUGH */
        case IMG_PPM_ASCII:
            channels = 3;
            break;
        case IMG_PGM_BIN: /* FALLTHROUGH */
        case IMG_PGM_ASCII:
            channels = 1;
            return NULL;
        default:
            break;
    }

    if (!fgets(line, sizeof(line), tmp_file)) {
        fclose(tmp_file);
        return NULL;
    }  /* Read PNM magic number*/

    do {
        if (!fgets(line, sizeof(line), tmp_file)) {
            fclose(tmp_file); return NULL;
        }
    } while (line[0] == '#'); /* Read width & height*/

    if (!sscanf(line, "%hu %hu", &w, &h) ) {
        fclose(tmp_file);
        return NULL;
    }

    do {
        if (!fgets(line, sizeof(line), tmp_file)) {
            fclose(tmp_file); return NULL;
        }
    } while (line[0] == '#'); /* Ignore depth info, typically 255*/

    long pos = ftell(tmp_file);
    fclose(tmp_file);

    imgfile = open(file, O_RDONLY);
    if (imgfile < 0)
        return NULL;

    lseek(imgfile, pos, SEEK_SET);

    img = img_create(w, h, channels);
    if (img == NULL) {
        close(imgfile);
        return NULL;
    }


    while ((bytesread = read(imgfile, chunk, sizeof(chunk))) > 0) {
        int total_bytes = bytesread + leftover_size;
        int valid_bytes = (total_bytes / channels) * channels;

        // Copy leftover from the previous iteration
        memmove(chunk + leftover_size, chunk, bytesread);
        memcpy(chunk, leftover, leftover_size);

        for (i = 0; i < valid_bytes; i += channels) {
            for (ch = 0; ch < channels; ch++) {
                pixel[ch] = chunk[i + ch];
            }
            if (addpixel(img, pixel, &curr_pos) < 0) {
                close(imgfile);
                img_free(img);
                return NULL;
            }
        }

        // Store leftover bytes
        leftover_size = total_bytes % channels;
        memcpy(leftover, chunk + valid_bytes, leftover_size);
    }
    close(imgfile);
    return img;
}

int8_t
img_getpx(ImagePtr img, uint16_t x, uint16_t y, uint8_t *pixel)
{
    uint8_t *p, i;
    CHECK_PTR(pixel, -1);

    if (x >= img->width || y >= img->height) {
        return -1;
    }

    p = IMG_PIXEL_PTR(img, x, y);
    for(i = 0; i < img->channels; i++) {
        pixel[i] = p[i];
    }

    return 1;
}

int8_t
img_setpx(ImagePtr img, uint16_t x, uint16_t y, uint8_t *pixel)
{
    uint8_t *p, i;
    CHECK_PTR(pixel, -1);

    if (x >= img->width || y >= img->height) {
        return -1;
    }

    p = IMG_PIXEL_PTR(img, x, y);

    for(i = 0; i < img->channels; i++) {
        p[i] = MIN(255, pixel[i]);
    }

    return 1;
}

ImagePtr
img_cpy(ImagePtr src)
{
    ImagePtr img = NULL;

    CHECK_COND(src == NULL, "", NULL);

    img = img_create(src->width, src->height, src->channels);
    CHECK_ALLOC(img);
    img->type = src->type;

    memcpy(img->data, src->data, src->height * src->stride);

    return img;
}

uint8_t
img_save(ImagePtr img, const char *file)
{
    if(img == NULL || file == NULL || strlen(file) < 1)
        return -1;

    switch (img->type) {
        case IMG_PPM_BIN: /* FALLTHROUGH */
        case IMG_PPM_ASCII:
        case IMG_PGM_ASCII:
        case IMG_PGM_BIN:
            img_savepnm(img , file);
            break;
        default:
            fprintf(stderr, "Error: Unknown ImageType!\n(function: %s, line %d, file %s)\n", __func__, __LINE__, __FILE__); \
            return -1;
    }
    return 1;
}

uint8_t
img_savepnm(ImagePtr img, const char *file)
{
    FILE *fp;
    uint32_t x, y;
    uint8_t *row, *pixel;

    CHECK_PTR(img, -1);

    fp = fopen(file, "wb");
    CHECK_PTR(fp, -1);

    // Write header
    fprintf(fp, "%s\n%d %d\n255\n", HEX_TO_ASCII(img->type), img->width, img->height);

    for(y = 0; y < img->height; y++) {
        row = &img->data[y * img->stride];
        for(x = 0; x < img->width; x++) {
            pixel = &row[x * img->channels];
            if (fwrite(pixel, 1, img->channels, fp) != img->channels) {
                fclose(fp);
                return -1;
            }
        }
    }
    fclose(fp);
    return 1;
}

void
img_free(ImagePtr img)
{
    if(img == NULL) {
        fprintf(stderr, "img is NULL!");
        return;
    }

    free(img->data);
    free(img);
}

void
img_print(ImagePtr img)
{
    uint16_t i, j, k;
    uint8_t pixel[] = {0, 0, 0, 0};
    if (img == NULL) {
        fprintf(stderr, "img is NULL!");
        return;
    }

    for(i = 0; i < img->height; i++) {
        for(j = 0; j < img->width; j++) {
            printf("{ ");
            img_getpx(img, j, i, pixel);
            for(k = 0; k < img->channels; k++) {
                printf("%hu ",pixel[k]);
            }
            printf("},");
        }
        printf("\n");
    }
}
/*
    TODO: Extend this function to support displaying images independently of the format.

    Currently, this implementation saves the image in PPM format and relies on an
    external image viewer to display it. To improve flexibility, consider:

    - Supporting multiple image formats instead of relying solely on a single format.
    - Using an image processing library (e.g., stb_image, ImageMagick) to handle 
      different formats.
    - Directly rendering images in a GUI window instead of invoking an external viewer.

*/


int8_t
img_disp(ImagePtr img, const char* imgviewer)
{
    int fd;
    char template[] = "/tmp/img_XXXXXX", CMD[0xFF];

    printf("Using viewer command: '%s'\n", imgviewer);

    fd = mkstemp(template);
    if(fd == -1) {
        fprintf(stderr, "Error creating temporary file\n");
        return -1;
    }

    FILE* file = fdopen(fd, "w+");
    if(file == NULL) {
        fprintf(stderr, "Error opening temporary image file\n");
        close(fd);
        unlink(template);
        return -1;
    }

    img_save(img, template);
    sprintf(CMD, "%s %s", imgviewer, template);
    printf("Executing command: '%s'\n", CMD);
    system(CMD);

    fclose(file);
    unlink(template);

    return 0;
}

Kernel kernel_alloc(KernelSize sz){
    Kernel kernel = {0};
    kernel.size = sz;
    kernel.data = (float*) malloc(kernel.size * kernel.size * sizeof(float));
    KERNEL_ASSERT(kernel);
    return kernel;
}

Kernel
img_get_kernel(KernelType type, KernelSize size)
{
    const float sobel_x[] = {
        -1.0f, 0.0f, 1.0f,
        -2.0f, 0.0f, 2.0f,
        -1.0f, 0.0f, 1.0f
    },
    sobel_y[] = {
        -1.0f, -2.0f, -1.0f,
        0.0f,  0.0f,  0.0f,
        1.0f,  2.0f,  1.0f
    };



    Kernel kernel = {0};
    float size_squared = size * size;
    size_t i, center = (size_t)size_squared/2;

    kernel = kernel_alloc(size);
    switch(type) {
        case KERNEL_IDENTITY:

            memset(kernel.data, 0, size_squared * sizeof(float));
            kernel.data[center] = 1.0f;
            break;

        case KERNEL_BOX_BLUR:

            for (i = 0; i < (size_squared); i++) {
                kernel.data[i] = 1.0f / size_squared ;
            }
            break;


        case KERNEL_SHARPEN:
            assert(size != IMG_KERNEL_3x3);

            /*TODO: implement a generalized funciton that creates a sharpen kernel of custom size n */
            float sharpen[] = {
                0.0f, -1.0f, 0.0f,
                -1.0f, 5.0f, -1.0f,
                0.0f, -1.0f, 0.0f
            };

            /*May be replaced by fill_kernel util ?, i don't know*/
            memcpy(kernel.data, sharpen, size_squared * sizeof(float));
            break;

        case KERNEL_SOBEL_X:

            memcpy(kernel.data, sobel_x, size_squared * sizeof(float));
            break;

        case KERNEL_SOBEL_Y:

            memcpy(kernel.data, sobel_y, size_squared * sizeof(float));
            break;

        case KERNEL_LAPLACIAN:
            /*TODO: implement a funciton that creates a laplacian kernel of custom size n */
            assert(size != IMG_KERNEL_3x3);

            float laplacian[] = {
                0.0f,  1.0f, 0.0f,
                1.0f, -4.0f, 1.0f,
                0.0f,  1.0f, 0.0f
            };
            memcpy(kernel.data, laplacian, size_squared * sizeof(float));
            break;

        case KERNEL_GAUSSIAN_BLUR:
            /*TODO: implement the gaussian formula */
        default:
            fprintf(stderr, "Unknown kernel type\n");
            kernel.size = 0;
            kernel.data = NULL;
    }

    return kernel;
}

int8_t
img_filter2D(ImagePtr img, KernelType type, KernelSize size, BorderMode border_mode)
{
    CHECK_PTR(img, -1);

    Kernel kernel = img_get_kernel(type, size);
    if (kernel.data == NULL || kernel.size == 0) {
        fprintf(stderr, "Failed to create kernel\n");
        return -1;
    }

    img_convolve(img, kernel, border_mode);

    img_free_kernel(kernel);

    return 1;
}

void
img_print_kernel(Kernel kernel)
{
    KERNEL_ASSERT(kernel);

    for (int i = 0; i < kernel.size; i++) {
        for (int j = 0; j < kernel.size; j++) {
            printf("%6.2f ", kernel.data[i * kernel.size + j]);
        }
        printf("\n");
    }
}



void
img_free_kernel(Kernel kernel)
{
    if (kernel.data) {
        free(kernel.data);
        kernel.data = NULL;
        kernel.size = 0;
    }
}

/* TODO: This is probably the worst implementation ever.
   Many things can be handled more efficiently.
*/
void img_convolve(ImagePtr img, Kernel kernel, BorderMode border_mode)
{
    uint8_t channels, *p;
    uint16_t x, y, half_kernel;
    int16_t kx, ky, px , py;
    double sum_r, sum_g, sum_b;
    float kernel_val;
    uint32_t offset;

    CHECK_COND(!img || !img->data || kernel.data == NULL|| kernel.size % 2 == 0,
               "Invalid input parameters for convolution.\n", );

    half_kernel = kernel.size /2;
    channels = img->channels;
    // Create a copy of the original image data for reference
    uint8_t *orig_data = (uint8_t *)malloc(img->height * img->stride);
    CHECK_PTR(orig_data,);
    memcpy(orig_data, img->data, img->height * img->stride);

    for (y = 0; y < img->height; y++) {
        for (x = 0; x < img->width; x++) {
            sum_r = 0.0, sum_g = 0.0, sum_b = 0.0;

            for (ky = -half_kernel; ky <= half_kernel; ky++) {
                for (kx = -half_kernel; kx <= half_kernel; kx++) {
                    px = x + kx;
                    py = y + ky;

                    if (px < 0 || px >= img->width || py < 0 || py >= img->height) {
                        switch(border_mode) {
                            case IMG_BORDER_ZERO_PADDING:
                                continue;
                            case IMG_BORDER_REPLICATE:
                                px = MIN(MAX(px, 0), img->width - 1);
                                py = MIN(MAX(py, 0), img->height - 1);
                        }
                    }

                    // Get kernel value
                    kernel_val = kernel.data[(ky + half_kernel) * kernel.size + (kx + half_kernel)];

                    // Get pixel from the original image copy
                    offset = py * img->stride + px * channels;
                    sum_r += orig_data[offset] * kernel_val;
                    if(channels == 3){
                        sum_g += orig_data[offset + 1] * kernel_val;
                        sum_b += orig_data[offset + 2] * kernel_val;
                    }
                }
            }

            sum_r = MIN(MAX(sum_r, 0.0), 255.0);
            if(channels == 3){
                sum_g = MIN(MAX(sum_g, 0.0), 255.0);
                sum_b = MIN(MAX(sum_b, 0.0), 255.0);
            }

            // Write output directly to the image
            p = IMG_PIXEL_PTR(img, x, y);
            p[0] = (uint8_t)sum_r;
            if(channels == 3){
                p[1] = (uint8_t)sum_g;
                p[2] = (uint8_t)sum_b;
            }
        }
    }

    // Free the reference copy
    free(orig_data);
}

ImagePtr
img_rgb2gray(ImagePtr img)
{
    ImagePtr newimg;
    uint16_t x,y;
    uint8_t pixel[4] = {0, 0, 0, 0}, newpixel[1] = {0};

    CHECK_PTR(img, NULL);

    newimg = (ImagePtr) malloc(sizeof(Image));
    CHECK_ALLOC(newimg);

    newimg->width = img->width;
    newimg->height = img->height;
    newimg->channels = 1;
    newimg->type = IMG_PGM_BIN;
    newimg->stride = calc_stride(newimg->width , 1);
    newimg->data = (uint8_t*) malloc(newimg->height * newimg->stride);

    CHECK_ALLOC(newimg->data);

    for(x = 0; x < newimg->width; ++x) {
        for(y = 0; y < newimg->height; ++y) {
            img_getpx(img, x, y, pixel);
            /* refernce for the formula: https://poynton.ca/PDFs/ColorFAQ.pdf */
            newpixel[0] = 0.2125 * pixel[0] + 0.7154 * pixel[1] + 0.0721 * pixel[2];
            img_setpx(newimg, x, y, newpixel);
        }
    }

    return newimg;
}

/* TODO: Find an efficient implementation for this cubic kernel */
static float
cubic_kernel(float x)
{
     // return (float)((P(x+2) * P(x+2) * P(x+2)) - 4 *(P(x+1) * P(x+1) * P(x+1)) + 6 * (P(x) * P(x) * P(x)) - 4 * (P(x - 1) * P(x - 1) * P(x - 1)))/6;
    x = ABS(x);
    if (x <= 1.0f)
        return (1.5f * x - 2.5f) * x * x + 1.0f;
    else if (x < 2.0f)
        return ((-0.5f * x + 2.5f) * x - 4.0f) * x + 2.0f;
    return 0.0f;
}

/*
    resize Using Bicubic Interpolation
    Reference: https://iopscience.iop.org/article/10.1088/1742-6596/1114/1/012066
*/
/* TODO: Optimize redundant calculations and memory access */
ImagePtr
img_resize(ImagePtr src, uint16_t new_width, uint16_t new_height)
{
    ImagePtr img;
    float scale_x, scale_y, src_x, src_y, value;
    int ix, iy, c, n, m;
    uint16_t  x, y;
    uint8_t pixel[3] = {0}, spixel[3];
    CHECK_PTR(src , NULL);

    img = img_create(new_width, new_height, src->channels);
    img->type = src->type;
    CHECK_PTR(img, NULL);

    scale_x = (float)src->width / new_width;
    scale_y = (float)src->height / new_height;

    for (y = 0; y < new_height; y++) {
        for (x = 0; x < new_width; x++) {
            src_x = (x + 0.5f) * scale_x - 0.5f;
            src_y = (y + 0.5f) * scale_y - 0.5f;
            ix = (int)FLOOR(src_x);
            iy = (int)FLOOR(src_y);

            for (c = 0; c < src->channels; c++) {
                value = 0.0f;
                for (m = -1; m <= 2; m++) {
                    for (n = -1; n <= 2; n++) {
                        int sx = ix + n;
                        int sy = iy + m;
                        if (sx < 0) sx = 0;
                        if (sy < 0) sy = 0;
                        sx = (sx >= src->width ? src->width - 1 : sx);
                        sy = (sy >= src->height ? src->height - 1 : sy);

                        img_getpx(src, sx, sy, spixel);
                        value += spixel[c] * cubic_kernel(n - (src_x - ix)) * cubic_kernel(m - (src_y - iy));
                    }
                }
                pixel[c] = MIN(MAX(value, 0), 255);
            }
            img_setpx(img, x, y, pixel);
        }
    }
    return img;
}


ImagePtr
img_add(ImagePtr img1, ImagePtr img2)
{
    ImagePtr img = NULL;
    uint16_t x, y, width, height, sum;
    uint8_t ch, channels, pixel1[] = {0, 0, 0, 0}, pixel2[] = {0, 0, 0, 0};


    CHECK_COND(img1 == NULL || img2 == NULL, "", NULL);
    CHECK_COND(img1->width != img2->width        ||
               img1->height != img2->height      ||
               img1->channels != img2->channels ||
               img1->type != img2->type,
               "",
               NULL);

    width = img1->width;
    height = img1->height;
    channels = img1->channels;

    img = img_create(width, height, channels);
    img->type = img1->type;
    CHECK_ALLOC(img);

    for(y = 0; y < height; ++y){
        for(x = 0; x < width; ++x){
            img_getpx(img1, x, y, pixel1);
            img_getpx(img2, x, y, pixel2);
            for(ch = 0; ch < channels; ++ch){
                sum = (uint16_t) pixel1[ch] + (uint16_t) pixel2[ch];
                pixel1[ch] = (uint8_t)(MIN(255, sum));
            }
            img_setpx(img, x, y, pixel1);
        }
    }
    return img;
}


ImagePtr
img_subtract(ImagePtr img1, ImagePtr img2)
{
    ImagePtr img = NULL;
    uint16_t x, y, width, height, diff;
    uint8_t ch, channels, pixel1[] = {0, 0, 0, 0}, pixel2[] = {0, 0, 0, 0}, pixel3[] = {0,0,0,0};


    CHECK_COND(img1 == NULL || img2 == NULL, "", NULL);
    CHECK_COND(img1->width != img2->width        ||
               img1->height != img2->height      ||
               img1->channels != img2->channels ||
               img1->type != img2->type,
               "",
               NULL);

    width = img1->width;
    height = img1->height;
    channels = img1->channels;

    img = img_create(width, height, channels);
    img->type = img1->type;
    CHECK_ALLOC(img);

    for(y = 0; y < height; ++y){
        for(x = 0; x < width; ++x){
            img_getpx(img1, x, y, pixel1);
            img_getpx(img2, x, y, pixel2);
            for(ch = 0; ch < channels; ++ch){
                 diff = ABS(pixel1[ch] - pixel2[ch]);
                pixel3[ch] = (diff < 0) ? 0 : diff;
            }
            img_setpx(img, x, y, pixel3);
        }
    }
    return img;
}



