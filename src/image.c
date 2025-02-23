#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>

#include "image.h"

#define MAXLINE 1024
#define CHUNK_SIZE 8192

#define IMG_PIXEL_PTR(img, x, y) ((uint8_t*)((img)->data + (y) * (img)->stride + (x) * (img)->channels))

/*TODO: find more flexible & dynamic way for this (more than 2 bytes))*/
#define HEX_TO_ASCII(hex) (char[]){(char)((hex) >> 8), (char)((hex) & 0xFF), '\0'}

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
    img = (ImagePtr) malloc( sizeof(image));
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
    CHECK_COND(file == NULL , "" ,NULL );

    ImagePtr img = NULL;
    ImgType type = img_type(file);

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
    unsigned int i;
    char magic[MAXLINE];
    uint64_t magic_;
    char line[MAXLINE];

    /* Read the PNM/PFM file header. */
    f = fopen(file, "rb");

    CHECK_PTR(f, IMG_UNKNOWN);
    while (fgets(line, MAXLINE, f) != NULL) {
        int flag = 0;
        for (i = 0; i < strlen(line); i++) {
            if (isgraph(line[i])) {
                if ((line[i] == '#') && (flag == 0)) {
                    flag = 1;
                }
            }
        }
        if (flag == 0) {
            sscanf(line, "%2s", magic);
            break;
        }
    }

    magic_ = (magic[0] << 8) | magic[1];

    switch(magic_) {
        case IMG_PPM_BIN:
            return IMG_PPM_BIN;
        case IMG_PPM_ASCII:
            return IMG_PPM_ASCII;
        case IMG_PGM_BIN:
            return IMG_PGM_BIN;
        case IMG_PGM_ASCII:
            return IMG_PGM_ASCII;
        default: return IMG_UNKNOWN;
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
            break;
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

    if (x >= img->width || y >= img->height) {
        return -1;
    }

    p = IMG_PIXEL_PTR(img, x, y);

    for(i = 0; i < img->channels; i++) {
        p[i] = pixel[i];
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

ImagePtr
img_rgb2gray(ImagePtr img)
{
    ImagePtr newimg;
    uint16_t x,y;
    uint8_t pixel[4] = {0, 0, 0, 0}, newpixel[1] = {0};

    CHECK_PTR(img, NULL);

    newimg = (ImagePtr) malloc(sizeof(image));
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

