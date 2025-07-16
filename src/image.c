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
/* TODO: find more flexible & dynamic way for this (more than 2 bytes))*/
#define HEX_TO_ASCII(hex)         (char[]){(char)((hex) >> 8), (char)((hex) & 0xFF), '\0'}
#define MUST(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Error: %s\n", (message)); \
            assert(condition); \
        } \
    } while (0)


#define ERROR(errcode, str)       {errcode, str}

struct error_entry {
    ImgError errcode;
    const char *str;
};

static const struct error_entry error_entries[] = {
    ERROR(IMG_OK,                      "Everything is okay!"),
    ERROR(IMG_ERR_FILE_NOT_FOUND,      "File not found"),
    ERROR(IMG_ERR_FILE_READ,           "Error reading from the file"),
    ERROR(IMG_ERR_FILE_WRITE,          "Error writing to the file"),
    ERROR(IMG_ERR_FILE_CREATE,         "Error creating the file"),
    ERROR(IMG_ERR_UNSUPPORTED_FORMAT,  "Unsupported image format"),
    ERROR(IMG_ERR_MEMORY,              "Memory allocation failed"),
    ERROR(IMG_ERR_INVALID_PARAMETERS,  "Parameters passed to the functions are invalid"),
    ERROR(IMG_ERR_INVALID_DIMENSIONS,  "Invalid dimensions passed"),
    ERROR(IMG_ERR_INVALID_KERNEL_SIZE, "Invalid kernel size"),
    ERROR(IMG_ERR_UNSUPPORTED_KERNEL,  "Unsupported Kernel type"),
    ERROR(IMG_ERR_CORRUPT_DATA,        "Corrupted image data"),
    ERROR(IMG_ERR_COLOR_SPACE,         "Unsupported or invalid color space"),
    ERROR(IMG_ERR_UNKNOWN,             "Unknown error")
};

static inline uint32_t
calc_stride(uint16_t width, uint8_t channels)
{
    return (((uint32_t) width * (uint32_t)channels + 15) & ~(uint32_t)15);
}

/* TODO: Replace current_pos logic with x, y to use img_getpx & img_setpx*/
static int8_t
addpixel(Image *img, const uint8_t *pixel, uint32_t *current_pos)
{
    uint8_t *p, i;
    uint32_t x, y;

    MUST(img != NULL,         "img is NULL in addpixel"); 
    MUST(pixel != NULL ,      "img is NULL in addpixel");
    MUST(current_pos != NULL, "img is NULL in addpixel"); 

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

ImgError
img_realloc_pixels(Image *img, uint16_t new_width, uint16_t new_height, uint8_t new_channels)
{
    ImgError err;
    MUST(img != NULL, "img is NULL in img_realloc_pixels");

    err = IMG_OK;
    if(new_width < 1 || new_height < 1 || new_channels < 1 || new_channels > 4){
        err = IMG_ERR_INVALID_DIMENSIONS;
        return err;
    }


    img->stride = calc_stride(new_width, new_channels);
    img->data = (uint8_t*) realloc(img->data, new_height * img->stride);
    MUST(img->data != NULL, "img->data is NULL in img_realloc_pixels");

    memset(img->data, 0, new_height * img->stride);

    img->width = new_width;
    img->height = new_height;
    img->channels = new_channels;

    return err;
}

ImgError
img_init(Image *img, uint16_t width, uint16_t height, uint8_t channels)
{
    ImgError err;

    /*TODO: Extend channel support beyond the current 1-4 limit to accommodate additional color spaces:
     * - LAB (3 channels)
     * - CMYK (4 channels)
     * - CMYKA (5 channels)
     * - etc.
     */
    if(width == 0 || height == 0 || channels < 1 || channels > 4){
        err = IMG_ERR_INVALID_DIMENSIONS; goto error;
    }

    // img = (Image *) malloc(sizeof(Image));
    // CHECK_ALLOC(img)

    img->stride = calc_stride(width, channels);
    img->data = (uint8_t*) malloc(height * img->stride);
    if(img->data == NULL) {
        err = IMG_ERR_MEMORY;  goto error;
    }

    memset(img->data, 0, height * img->stride);

    img->width = width;
    img->height = height;
    img->channels = channels;
    img->type = -1;

    err = IMG_OK;
error:
    return err;
}

ImgError
img_type(const char *file, ImgType *type)
{
    ImgError err;
    FILE* f;
    char magic[3] = {0};
    char line[MAXLINE];

    MUST(file != NULL, "file is NULL in img_type");
    MUST(type != NULL, "type is NULL in img_type");

    err = IMG_OK;

    f = fopen(file, "rb");
    if(f == NULL){
        err = IMG_ERR_FILE_NOT_FOUND;
        return err;
    }

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
            *type = IMG_PPM_BIN;
            break;
        case IMG_PPM_ASCII:
            *type = IMG_PPM_ASCII;
            break;
        case IMG_PGM_BIN:
            *type = IMG_PGM_BIN;
            break;
        case IMG_PGM_ASCII:
            *type = IMG_PGM_ASCII;
            break;
        default:
            *type =  IMG_UNKNOWN;
            break;
    }
    return err;
}

ImgError
img_load(Image *img, const char* file)
{
    ImgError err;
    FILE *f;
    ImgType type;
    MUST(img != NULL, "img is NULL in img_load");

    err = IMG_OK;

    f = fopen(file, "r");
    if (f == NULL){
        err = IMG_ERR_FILE_NOT_FOUND;
        return err;
    }

    err = img_type(file, &type);
    if(err != IMG_OK){
        return err;
    }

    switch(type) {
        case IMG_PPM_BIN: /* FALLTHROUGH */
        case IMG_PPM_ASCII:
        case IMG_PGM_BIN:
        case IMG_PGM_ASCII:
            err = img_loadpnm(img, file, type);
            break;
        default:
            err = IMG_ERR_UNSUPPORTED_FORMAT;
            return err;
    }

    return err;
}


/* TODO: Optimize this funciton*/
ImgError
img_loadpnm(Image *img, const char* file, ImgType type)
{
    ImgError err;
    ssize_t imgfile;
    FILE* tmp_file;
    uint8_t pixel[3], channels, ch, leftover[2] = {0}, leftover_size = 0;
    uint16_t w, h;
    uint32_t i, curr_pos = 0, bytesread = 0;
    char line[MAXLINE], chunk[CHUNK_SIZE];

    tmp_file = fopen(file, "rb");
    if(tmp_file  == NULL){
        err = IMG_ERR_FILE_NOT_FOUND;
        return err;
    } 

    switch(type) {
        case IMG_PPM_BIN: /* FALLTHROUGH */
        case IMG_PPM_ASCII:
            channels = 3;
            break;
        case IMG_PGM_BIN: /* FALLTHROUGH */
        case IMG_PGM_ASCII:
            channels = 1;
            break;
        default:
            err = IMG_ERR_UNSUPPORTED_FORMAT;
            return err;
    }

    if (!fgets(line, sizeof(line), tmp_file)) {
        fclose(tmp_file);
        err = IMG_ERR_FILE_READ;
        return err;
    }  /* Read PNM magic number*/

    do {
        if (!fgets(line, sizeof(line), tmp_file)) {
            fclose(tmp_file); 
            err = IMG_ERR_FILE_READ;
            return err;
        }
    } while (line[0] == '#'); /* Read width & height*/

    if (!sscanf(line, "%hu %hu", &w, &h) ) {
        fclose(tmp_file);
        err = IMG_ERR_FILE_READ; 
        return err;
    }

    do {
        if (!fgets(line, sizeof(line), tmp_file)) {
            fclose(tmp_file);
            err = IMG_ERR_FILE_READ;
            return err;
        }
    } while (line[0] == '#'); /* Ignore depth info, typically 255*/

    long pos = ftell(tmp_file);
    fclose(tmp_file);

    imgfile = open(file, O_RDONLY);
    if(imgfile < 0){
        err = IMG_ERR_FILE_READ; 
        return err;
    } 

    lseek(imgfile, pos, SEEK_SET);

    err = img_init(img, w, h, channels);
    img->type = type;
    if(err != IMG_OK) {
        close(imgfile);
        return err;
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
                err = IMG_ERR_CORRUPT_DATA; 
                return err;
            }
        }

        // Store leftover bytes
        leftover_size = total_bytes % channels;
        memcpy(leftover, chunk + valid_bytes, leftover_size);
    }
    close(imgfile);

    err = IMG_OK;
    return err;
}

ImgError
img_getpx(Image *img, uint16_t x, uint16_t y, uint8_t *pixel)
{
    uint8_t *p, i;
    ImgError err;

    err = IMG_OK;
    if (img == NULL || pixel == NULL || x >= img->width || y >= img->height) {
        fprintf(stderr, "Error: (function: %s, line %d, file %s)\n", __func__, __LINE__, __FILE__);
        err = IMG_ERR_INVALID_PARAMETERS;
        return err;
    }

    p = IMG_PIXEL_PTR(img, x, y);
    for(i = 0; i < img->channels; i++) {
        pixel[i] = p[i];
    }

    return 1;
}

ImgError
img_setpx(Image *img, uint16_t x, uint16_t y, uint8_t *pixel)
{
    uint8_t *p, i;
    ImgError err;

    err = IMG_OK;
    if (img == NULL || pixel == NULL || x >= img->width || y >= img->height) {
        fprintf(stderr, "Error: (function: %s, line %d, file %s)\n", __func__, __LINE__, __FILE__);
        err = IMG_ERR_INVALID_PARAMETERS;
        return err;
    }

    p = IMG_PIXEL_PTR(img, x, y);

    for(i = 0; i < img->channels; i++) {
        p[i] = MIN(255, pixel[i]);
    }

    return err;
}

ImgError
img_cpy(Image *dest, Image *src)
{
    ImgError err;

    MUST(dest != NULL, "dest is NULL in img_cpy");
    MUST(src  != NULL, "src is NULL in img_cpy");
    MUST(src->data  != NULL, "src->data is NULL in img_cpy");

    err = IMG_OK;
    /* Check if destination has compatible dimensions and channels */
    if (dest->width != src->width || 
        dest->height != src->height || 
        dest->channels != src->channels) {

        if (dest->data != NULL) {
            free(dest->data);
        }

        // Reinitialize destination with source dimensions
        err = img_init(dest, src->width, src->height, src->channels);
        if (err != IMG_OK) {
            return err;
        }
    }

    dest->type = src->type;

    // Copy image data
    memcpy(dest->data, src->data, src->height * src->stride);

    return err;
}

ImgError
img_save(Image *img, const char *file)
{
    ImgError err;
    MUST(img != NULL, "img is NULL in img_save");
    MUST(file != NULL, "file is NULL in img_save");

    if(strlen(file) < 1){
        err = IMG_ERR_INVALID_PARAMETERS;
        return err;
    }

    switch (img->type) {
        case IMG_PPM_BIN: /* FALLTHROUGH */
        case IMG_PPM_ASCII:
        case IMG_PGM_ASCII:
        case IMG_PGM_BIN:
            err = img_savepnm(img , file);
            break;
        default:
            err = IMG_ERR_UNSUPPORTED_FORMAT;
    }
    return err;
}

ImgError
img_savepnm(Image *img, const char *file)
{
    ImgError err;
    FILE *fp;
    uint32_t x, y;
    uint8_t *row, *pixel;

    MUST(img != NULL, "img is NULL in img_savepnm");
    MUST(file != NULL, "file is NULL in img_savepnm");

    err = IMG_OK;

    fp = fopen(file, "wb");
    if(fp == NULL){
        err = IMG_ERR_FILE_READ;
        return err;
    }

    // Write header
    fprintf(fp, "%s\n%d %d\n255\n", HEX_TO_ASCII(img->type), img->width, img->height);

    for(y = 0; y < img->height; y++) {
        row = &img->data[y * img->stride];
        for(x = 0; x < img->width; x++) {
            pixel = &row[x * img->channels];
            if (fwrite(pixel, 1, img->channels, fp) != img->channels) {
                fclose(fp);
                err = IMG_ERR_FILE_WRITE;
                return err;
            }
        }
    }
    fclose(fp);
    return err;
}

void
img_free(Image *img)
{
    MUST(img       != NULL, "img is NULL in img_free");
    MUST(img->data != NULL, "img->data is NULL in img_free");

    free(img->data);
}

void
img_print(Image *img)
{
    uint16_t i, j, k;
    uint8_t pixel[] = {0, 0, 0, 0};
    MUST(img       != NULL, "img is NULL in img_print");
    MUST(img->data != NULL, "img->data is NULL in img_print");

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


ImgError
img_disp(Image *img, const char* imgviewer)
{
    ImgError err;
    int fd;
    char template[] = "/tmp/img_XXXXXX", CMD[0xFF];
    err = IMG_OK;
    printf("Using viewer command: '%s'\n", imgviewer);

    fd = mkstemp(template);
    if(fd == -1) {
        err = IMG_ERR_FILE_CREATE;
        return err;
    }

    FILE* file = fdopen(fd, "w+");
    if(file == NULL) {
        close(fd);
        unlink(template);
        err = IMG_ERR_FILE_READ;
        return err;
    }

    err = img_save(img, template);
    if(err != IMG_OK){
        return err;
    }

    sprintf(CMD, "%s %s", imgviewer, template);
    printf("Executing command: '%s'\n", CMD);
    system(CMD);

    fclose(file);
    unlink(template);

    return err;
}


const char *
img_strerror(char *buf, size_t sz, ImgError status)
{
    size_t i;
    const struct error_entry* entry = NULL;
    for(i = 0; i < IMG_ARR_SIZE(error_entries); ++i ){
        if(status == error_entries[i].errcode){
            entry = &error_entries[i];
        }
    }
    strncpy(buf, entry->str, sz);
    return buf;
}

ImgError 
kernel_alloc(KernelSize sz, Kernel *kernel)
{
    ImgError err;
    MUST(kernel != NULL, "kernel is NULL in kernel_alloc");

    err = IMG_OK;
    kernel->size = sz;

    kernel->data = (float*) malloc(kernel->size * kernel->size * sizeof(float));
    MUST(kernel->data != NULL, "kernel->data is NULL in kernel_alloc");
    return err;
}

ImgError
img_get_kernel(KernelType type, KernelSize size, Kernel *kernel)
{
    ImgError err;
    const float sobel_x[] = {
        -1.0f, 0.0f, 1.0f,
        -2.0f, 0.0f, 2.0f,
        -1.0f, 0.0f, 1.0f
    },
    sobel_y[] = {
        -1.0f, -2.0f, -1.0f,
        0.0f,  0.0f,  0.0f,
        1.0f,  2.0f,  1.0f
    },
    sharpen[] = {
        0.0f, -1.0f, 0.0f,
        -1.0f, 5.0f, -1.0f,
        0.0f, -1.0f, 0.0f
    };

    float size_squared = size * size;
    size_t i, center = (size_t)size_squared/2;
    err = IMG_OK;

    err = kernel_alloc(size, kernel);
    if(err != IMG_OK){
        return err;
    }
    switch(type) {
        case IMG_KERNEL_IDENTITY:

            memset(kernel->data, 0, size_squared * sizeof(float));
            kernel->data[center] = 1.0f;
            break;

        case IMG_KERNEL_BOX_BLUR:

            for (i = 0; i < (size_squared); i++) {
                kernel->data[i] = 1.0f / size_squared ;
            }
            break;


        case IMG_KERNEL_SHARPEN:

            /*TODO: implement a generalized funciton that creates a sharpen kernel of custom size n */

            /*May be replaced by fill_kernel util ?, i don't know*/
            memcpy(kernel->data, sharpen, size_squared * sizeof(float));
            break;

        case IMG_KERNEL_SOBEL_X:

            memcpy(kernel->data, sobel_x, size_squared * sizeof(float));
            break;

        case IMG_KERNEL_SOBEL_Y:

            memcpy(kernel->data, sobel_y, size_squared * sizeof(float));
            break;

        case IMG_KERNEL_LAPLACIAN:
            /*TODO: implement a funciton that creates a laplacian kernel of custom size n */
            assert(size == IMG_KERNEL_3x3);

            float laplacian[] = {
                0.0f,  1.0f, 0.0f,
                1.0f, -4.0f, 1.0f,
                0.0f,  1.0f, 0.0f
            };
            memcpy(kernel->data, laplacian, size_squared * sizeof(float));
            break;

        case IMG_KERNEL_GAUSSIAN_BLUR:
            /*TODO: implement the gaussian formula */
        default:
            err = IMG_ERR_MEMORY;
            break;
    }

    return err;
}

ImgError
img_filter2D(Image *dest, Image *img, KernelType type, KernelSize size, BorderMode border_mode)
{
    ImgError err;
    Kernel kernel = {0};
    MUST(img       != NULL, "img is NULL in img_filter2D");
    MUST(img->data != NULL, "img->data is NULL in img_filter2D");
    MUST(dest      != NULL, "dest is NULL in img_filter2D");

    err = img_get_kernel(type, size, &kernel);
    if (err != IMG_OK) {
        return err;
    }

    err = img_convolve(dest, img, &kernel, border_mode);
    if (err != IMG_OK) {
        return err;
    }

    img_free_kernel(&kernel);

    return err;
}

ImgError
img_print_kernel(Kernel *kernel)
{
    ImgError err;
    MUST(kernel       != NULL, "kernel is NULL in img_print_kernel");
    MUST(kernel->data != NULL, "kernel is NULL in img_print_kernel");

    err = IMG_OK;
    for (int i = 0; i < kernel->size; i++) {
        for (int j = 0; j < kernel->size; j++) {
            printf("%6.2f ", kernel->data[i * kernel->size + j]);
        }
        printf("\n");
    }
    return err;
}



void
img_free_kernel(Kernel *kernel)
{
    MUST(kernel       != NULL, "kernel is NULL in img_free_kernel");
    MUST(kernel->data != NULL, "kernel is NULL in img_free_kernel");

    free(kernel->data);
    kernel->data = NULL;
    kernel->size = 0;
}

/* TODO: This is probably the worst implementation ever.
   Many things can be handled more efficiently.
*/
ImgError 
img_convolve(Image *dest, Image *img, Kernel *kernel, BorderMode border_mode)
{
    ImgError err;
    uint8_t channels, *p;
    uint16_t x, y, half_kernel;
    int16_t kx, ky, px , py;
    double sum_r, sum_g, sum_b;
    float kernel_val;
    uint32_t offset;

    MUST(img             != NULL, "img is NULL in img_convolve");
    MUST(img->data       != NULL, "img->data is NULL in img_convolve");
    MUST(kernel->data     != NULL, "kernel->data is NULL in img_convolve");
    MUST(kernel->size % 2 != 0,    "kernel->size % 2 == 0 NULL in img_convolve");

    err = IMG_OK;
    half_kernel = kernel->size /2;
    channels = img->channels;

    err = img_cpy(dest, img);
    if(err != IMG_OK){
        return err;
    }

    for (y = 0; y < dest->height; y++) {
        for (x = 0; x < dest->width; x++) {
            sum_r = 0.0, sum_g = 0.0, sum_b = 0.0;

            for (ky = -half_kernel; ky <= half_kernel; ky++) {
                for (kx = -half_kernel; kx <= half_kernel; kx++) {
                    px = x + kx;
                    py = y + ky;

                    if (px < 0 || px >= dest->width || py < 0 || py >= dest->height) {
                        switch(border_mode) {
                            case IMG_BORDER_ZERO_PADDING:
                                continue;
                            case IMG_BORDER_REPLICATE:
                                px = MIN(MAX(px, 0), dest->width - 1);
                                py = MIN(MAX(py, 0), dest->height - 1);
                        }
                    }

                    // Get kernel value
                    kernel_val = kernel->data[(ky + half_kernel) * kernel->size + (kx + half_kernel)];

                    // Get pixel from the img
                    offset = py * dest->stride + px * channels;
                    sum_r += img->data[offset] * kernel_val;
                    if(channels == 3){
                        sum_g += img->data[offset + 1] * kernel_val;
                        sum_b += img->data[offset + 2] * kernel_val;
                    }
                }
            }

            sum_r = MIN(MAX(sum_r, 0.0), 255.0);
            if(channels == 3){
                sum_g = MIN(MAX(sum_g, 0.0), 255.0);
                sum_b = MIN(MAX(sum_b, 0.0), 255.0);
            }

            // Write output directly to the image
            p = IMG_PIXEL_PTR(dest, x, y);
            p[0] = (uint8_t)sum_r;
            if(channels == 3){
                p[1] = (uint8_t)sum_g;
                p[2] = (uint8_t)sum_b;
            }
        }
    }
    return err;
}

ImgError
img_rgb2gray(Image *dest, Image *img)
{
    ImgError err;
    uint16_t x,y;
    uint8_t pixel[4] = {0, 0, 0, 0}, newpixel[1] = {0};

    MUST(dest      != NULL, "dest is NULL in img_rgb2gray");
    MUST(img       != NULL, "img is NULL in img_rgb2gray");
    MUST(img->data != NULL, "img->data is NULL in img_rgb2gray");

    err = img_realloc_pixels(dest, img->width, img->height, 1);
    if(err != IMG_OK){
        return err;
    }

    dest->type = IMG_PGM_BIN;
    for(x = 0; x < dest->width; ++x) {
        for(y = 0; y < dest->height; ++y) {
            img_getpx(img, x, y, pixel);
            /* refernce for the formula: https://poynton.ca/PDFs/ColorFAQ.pdf */
            newpixel[0] = 0.2125 * pixel[0] + 0.7154 * pixel[1] + 0.0721 * pixel[2];
            img_setpx(dest, x, y, newpixel);
        }
    }

    return err;
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
ImgError
img_resize(Image *dest, Image *src, uint16_t new_width, uint16_t new_height)
{
    ImgError err;
    float scale_x, scale_y, src_x, src_y, value;
    int ix, iy, c, n, m;
    uint16_t  x, y;
    uint8_t pixel[3] = {0}, spixel[3];

    err = IMG_OK;
    MUST(dest != NULL, "dest is NULL in img_resize");
    MUST(src  != NULL, "src is NULL in img_resize");

    if(new_width < 1 || new_height < 1){
        err = IMG_ERR_INVALID_PARAMETERS;
        return err;
    }

    err = img_realloc_pixels(dest, new_width, new_height, src->channels);
    dest->type = src->type;

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
            img_setpx(dest, x, y, pixel);
        }
    }
    return err;
}


ImgError
img_add(Image *dest, Image *img1, Image *img2)
{
    ImgError err;
    uint16_t x, y, width, height, sum;
    uint8_t ch, channels, pixel1[] = {0, 0, 0, 0}, pixel2[] = {0, 0, 0, 0};

    MUST(img1       != NULL, "img1 is NULL in img_add");
    MUST(img2       != NULL, "img1 is NULL in img_add");
    MUST(img1->data != NULL, "img1->data is NULL in img_add");
    MUST(img2->data != NULL, "img1->data is NULL in img_add");
    MUST(dest       != NULL, "dest is NULL in img_add");

    err = IMG_OK;
    if(img1->width != img2->width       ||
       img1->height != img2->height     ||
       img1->channels != img2->channels ||
       img1->type != img2->type
    ){
        err = IMG_ERR_INVALID_DIMENSIONS;
        return err;
    }

    width = img1->width;
    height = img1->height;
    channels = img1->channels;

    err = img_realloc_pixels(dest, width, height, channels);
    dest->type = img1->type;
    if(err != IMG_OK){
        return err;
    }


    for(y = 0; y < height; ++y){
        for(x = 0; x < width; ++x){
            img_getpx(img1, x, y, pixel1);
            img_getpx(img2, x, y, pixel2);
            for(ch = 0; ch < channels; ++ch){
                sum = (uint16_t) pixel1[ch] + (uint16_t) pixel2[ch];
                pixel1[ch] = (uint8_t)(MIN(255, sum));
            }
            img_setpx(dest, x, y, pixel1);
        }
    }
    return err;
}

/*
    https://homepages.inf.ed.ac.uk/rbf/HIPR2/pixsub.htm
    TODO: implement multiple subtraction methods (direct subtraction, absolute difference, and wrapped values)
    and allow the user to select their preferred method
*/
ImgError
img_subtract(Image *dest, Image *img1, Image *img2)
{
    ImgError err;
    uint16_t x, y, width, height;
    uint8_t ch, diff, channels, pixel1[] = {0, 0, 0, 0}, pixel2[] = {0, 0, 0, 0};

    MUST(img1       != NULL, "img1 is NULL in img_add");
    MUST(img2       != NULL, "img1 is NULL in img_add");
    MUST(img1->data != NULL, "img1->data is NULL in img_add");
    MUST(img2->data != NULL, "img1->data is NULL in img_add");
    MUST(dest       != NULL, "dest is NULL in img_add");

    err = IMG_OK;
    if(img1->width != img2->width       ||
       img1->height != img2->height     ||
       img1->channels != img2->channels ||
       img1->type != img2->type
    ){
        err = IMG_ERR_INVALID_DIMENSIONS;
        return err;
    }

    width = img1->width;
    height = img1->height;
    channels = img1->channels;

    err = img_realloc_pixels(dest, width, height, channels);
    dest->type = img1->type;
    if(err != IMG_OK){
        return err;
    }


    for(y = 0; y < height; ++y){
        for(x = 0; x < width; ++x){
            img_getpx(img1, x, y, pixel1);
            img_getpx(img2, x, y, pixel2);
            for(ch = 0; ch < channels; ++ch){
                diff = pixel1[ch] - pixel2[ch];
                pixel2[ch] = MAX(0, diff);
            }
            img_setpx(dest, x, y, pixel2);
        }
    }
    return err;
}
