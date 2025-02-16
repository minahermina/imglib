#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>

#include "image.h"

#define MAXLINE 1024
#define CHUNK_SIZE 8193

#define IMG_PIXEL_PTR(img, x, y) ((uint8_t*)((img)->data + (y) * (img)->stride + (x) * (img)->channels))

/*TODO: find more flexible & dynamic way for this (more than 2 bytes))*/
#define HEX_TO_ASCII(hex) (char[]){(char)((hex) >> 8), (char)((hex) & 0xFF), '\0'}

#define CHECK_ALLOC(ptr) \
    if (ptr != NULL) { \
        fprintf(stderr, "Memory allocation failed: %s (line %d)\n", __FILE__, __LINE__); \
        return NULL; \
    }

#define CHECK_PTR(ptr) \
    if (ptr == NULL) { \
        fprintf(stderr, "Error: %s is NULL! (line %d, file %s)\n", #ptr, __LINE__, __FILE__); \
        return NULL; \
    }


static inline uint32_t
calc_stride(uint16_t width, uint8_t channels) {
    return (((uint32_t) width * (uint32_t)channels + 15) & ~(uint32_t)15);
}

ImagePtr 
createimg(uint16_t width, uint16_t height, uint8_t channels)
{
    ImagePtr img;
    img = (ImagePtr) calloc(1, sizeof(image));
    CHECK_ALLOC(img)

    img->stride = calc_stride(width, channels);
    img->data = (uint8_t*) malloc(height * img->stride);
    CHECK_PTR(img->data);

    memset(img->data, 0, height * img->stride);

    img->width = width;
    img->height = height;
    img->channels = channels;
    img->type = -1;

    return img;
}


ImgType 
imgtype(const char *file)
{
    FILE* f = fopen(file, "rb");
    unsigned int i;
    char magic[MAXLINE];
    uint64_t magic_;
    char line[MAXLINE];

    /* Read the PNM/PFM file header. */
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

int8_t
addpixel(ImagePtr img, const uint8_t *pixel, uint32_t *current_pos)
{
    uint8_t *p, i;
    uint32_t x, y;

    if (img == NULL || pixel == NULL || current_pos == NULL) {
        fprintf(stderr, "Error: Invalid input (img, pixel, or position is NULL).\n");
        return -1;
    }

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

/* TODO: create a fucntion called loadpnm to handle PPM && PGM formats*/
ImagePtr
loadppm(const char* file)
{
    ImagePtr img;
    ssize_t imgfile;
    FILE* tmp_file;
    uint8_t pixel[3], channels, ch;
    uint16_t w, h;
    uint32_t i, curr_pos, bytesread;
    char line[MAXLINE], chunk[CHUNK_SIZE];

    tmp_file = fopen(file, "rb");
    if (tmp_file == NULL)
        return NULL;

    /* Read header */
    fgets(line, sizeof(line), tmp_file);
    fgets(line, sizeof(line), tmp_file);
    sscanf(line, "%hu %hu", &w, &h);
    fgets(line, sizeof(line), tmp_file);

    long pos = ftell(tmp_file);
    fclose(tmp_file);

    imgfile = open(file, O_RDONLY);
    if (imgfile < 0)
        return NULL;

    lseek(imgfile, pos, SEEK_SET);

    channels = 3;
    img = createimg(w, h, channels);
    if (img == NULL) {
        close(imgfile);
        return NULL;
    }

    bytesread = 0, curr_pos = 0;

    while((bytesread = read(imgfile, chunk, sizeof(chunk))) > 0){
        for(i = 0; i < bytesread; i += channels) {
            for(ch = 0; ch < channels; ch++) {
                pixel[ch] = chunk[i + ch];
            }
            if(addpixel(img, pixel, &curr_pos) < 0) {
                close(imgfile);
                freeimg(img);
                return NULL;
            }
        }
    }

    close(imgfile);
    return img;
}


uint8_t
savepnm(ImagePtr img, const char *file)
{
    FILE *fp;
    uint32_t x, y;
    uint8_t *row, *pixel;

    if (img == NULL) {
        fprintf(stderr, "img is NULL!");
        return -1;
    }

    fp = fopen(file, "wb");
    if (!fp)
        return -1;

    // Write header
    fprintf(fp, "%s\n%d %d\n255\n", HEX_TO_ASCII(img->type), img->width, img->height);

    for(y = 0; y < img->height; y++) {
        row = &img->data[y * img->stride];
        for(x = 0; x < img->width; x++) {
            pixel = &row[x * img->channels];
            if (fwrite(pixel, 1, img->channels, fp) != img->channels) {
                fclose(fp);
                return 0;
            }
        }
    }
    fclose(fp);
    return 1;
}


uint8_t
saveimg( ImagePtr img, const char *file)
{
    if(img == NULL || file == NULL || strlen(file) < 1) 
        return -1;

    switch (img->type) {
        case IMG_PPM_BIN:
        case IMG_PPM_ASCII:
        case IMG_PGM_ASCII:
        case IMG_PGM_BIN:
            savepnm(img , file);
            break;
        default:
            return -1;
    }
    return 1;
}


void
freeimg(ImagePtr img)
{
    free(img->data);
    free(img);
}


int8_t
getpixel(ImagePtr img, uint16_t x, uint16_t y, uint8_t *pixel)
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
setpixel(ImagePtr img, uint16_t x, uint16_t y, uint8_t *pixel)
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
loadimg(const char* file)
{
    if(strlen(file) < 1 ) 
        return NULL;

    ImagePtr img = NULL;
    int64_t type = imgtype(file);

    switch(type) {
        case IMG_PPM_BIN:
        case IMG_PPM_ASCII:
            img = loadppm(file);
            if (img){
                img->type = type;
            }else{
                fprintf(stderr, "Error loading image: %s\n", file);
                return NULL;
            }

            break;
        case IMG_PGM_BIN:
        case IMG_PGM_ASCII:
            break;
        default:
            fprintf(stderr, "Unsupported or invalid image format\n");
    }

    return img;
}

void
printimg(ImagePtr img)
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
            getpixel(img, j, i, pixel);
            for(k = 0; k < img->channels; k++) 
                printf("%hu ",pixel[k]);
            printf("},");
        }
        printf("\n");
    }
}

int8_t
dispimg(ImagePtr img, const char* imgviewer)
{
    char template[] = "/tmp/img_XXXXXX";
    char CMD[0xFF];
    int fd;


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

    saveimg(img, template);
    sprintf(CMD, "%s %s", imgviewer, template);
    printf("Executing command: '%s'\n", CMD);
    system(CMD);

    fclose(file);
    unlink(template);

    return 0;
}


ImagePtr
rgb2gray(ImagePtr img)
{
    ImagePtr newimg;
    uint16_t x,y;
    uint8_t pixel[4] = {0, 0, 0, 0}, newpixel[1] = {0};

    CHECK_PTR(img);

    newimg = (ImagePtr) malloc(sizeof(image));
    CHECK_ALLOC(newimg);

    newimg->width = img->width;
    newimg->height = img->height;
    newimg->channels = 1;
    newimg->type = IMG_PGM_BIN;
    newimg->stride = calc_stride(newimg->width , 1);
    newimg->data = (uint8_t*) malloc(newimg->height * newimg->stride);

    CHECK_ALLOC(newimg->data);

    for(x = 0; x < newimg->width; ++x){
        for(y = 0; y < newimg->height; ++y){
            getpixel(img, x, y, pixel);
            newpixel[0] = 0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2];
            setpixel(newimg, x, y, newpixel);
        }
    }

    return newimg;
}

