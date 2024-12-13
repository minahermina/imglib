#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "image.h"

#define MAX_LINE_LENGTH 200

#define IMG_PIXEL_PTR(img, x, y) ((uint8_t*)((img)->data + (y) * (img)->stride + (x) * (img)->channels))

static inline uint32_t 
calc_stride(uint16_t width, uint8_t channels) {
    return (((uint32_t) width * (uint32_t)channels + 15) & ~(uint32_t)15);
}

ImagePtr 
createimg(uint16_t width, uint16_t height, uint8_t channels)
{
    ImagePtr img;
    img = (ImagePtr) calloc(1, sizeof(image));
    if(img == NULL) {
        fprintf(stderr, "Buy More RAM LOL!\n");
        return NULL;
    }

    img->stride = calc_stride(width, channels);
    img->data = (uint8_t*) malloc(height * img->stride);
    if(img->data == NULL) {
        fprintf(stderr, "Buy more RAM LOL!\n");
        free(img);
        return NULL;
    }

    memset(img->data, 0, height * img->stride);

    img->width = width;
    img->height = height;
    img->channels = channels;
    img->type = -1;

    return img;
}


/* TODO : Handle other image formats: PNG, JPG */
ImgType 
imgtype(const char *file)
{
      int fd;
    ssize_t bytesread;
    unsigned char magic[8];  // Buffer for magic numbers
    uint16_t magic_num;
    
    fd = open(file, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error opening file %s\n%s", file, strerror(errno));
        return IMG_UNKNOWN;
    }

    bytesread = read(fd, magic, sizeof(magic));
    close(fd);
    
    if (bytesread < 2) {
        return IMG_UNKNOWN;
    }

    magic_num = (magic[0] << 8) | magic[1];
    
    switch(magic_num) {
        case 0x5036: return IMG_PPM_BIN;
        case 0x5033: return IMG_PPM_ASCII;
        case 0x5035: return IMG_PGM_BIN;
        case 0x5032: return IMG_PGM_ASCII;
        case 0x8950: return IMG_PNG;
        case 0xFFD8: return IMG_JPG;
        case 0x424D: return IMG_BMP;
        case 0x4749: return IMG_GIF;
        case 0x4949:
        case 0x4D4D: return IMG_TIFF;
        default: return IMG_UNKNOWN;
    }
}

/* TODO : handle other PPM formats (PGM)*/
ImagePtr 
loadppm(const char* file)
{
    ImagePtr img;
    FILE* imagefile;
    uint8_t *pixel, *row, channels, ch;
    uint16_t w, h;
    uint32_t j, i;
    char line[MAX_LINE_LENGTH];
    unsigned char buf[3];

    imagefile = fopen(file, "rb");
    if (imagefile == NULL)
        return NULL;

    /* Read header */
    fgets(line, sizeof(line), imagefile);
    fgets(line, sizeof(line), imagefile);
    sscanf(line, "%hu %hu", &w, &h);
    fgets(line, sizeof(line), imagefile);

    channels = 3;
    img = createimg(w, h, channels);
    if (!img) {
        fclose(imagefile);
        return NULL;
    }
    /* Load image pixels */
    for(i = 0; i < h; i++) {
        row = &img->data[i * img->stride];
        for(j = 0; j < w; j++) {
            if (fread(buf, 1, 3, imagefile) != 3) {
                freeimg(img);
                fclose(imagefile);
                return NULL;
            }
            pixel = &row[j * channels];
            for(ch = 0; ch < channels; ch++) {
                pixel[ch] = buf[ch];
            }
        }
    }

    fclose(imagefile);
    return img;
}

uint8_t 
saveppm(const char *file, ImagePtr img)
{
    FILE *fp;
    uint32_t x, y;
    uint8_t *row, *pixel;

    if (img == NULL) {
        fprintf(stderr, "img is NULL!");
        return 0;
    }

    fp = fopen(file, "wb");
    if (!fp)
        return 0;

    // Write header
    fprintf(fp, "P6\n%d %d\n255\n", img->width, img->height);

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


void 
freeimg(ImagePtr img)
{
    free(img->data);
    free(img);
}


int8_t 
getpixel(ImagePtr img, uint16_t x, uint16_t y, uint16_t *pixel)
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
setpixel(ImagePtr img, uint16_t x, uint8_t y, uint8_t *pixel)
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
            if (img) img->type = type;
            break;
        case IMG_PGM_BIN:
        case IMG_PGM_ASCII:
            // TODO: implement PGM loading
            fprintf(stderr, "PGM format not yet supported\n");
            break;
        default:
            fprintf(stderr, "Unsupported or invalid image format\n");
    }
    
    return img;
}
void 
printimg(ImagePtr img)
{
    uint16_t index, i, j;
    uint8_t k;
    if (img == NULL) {
        fprintf(stderr, "img is NULL!");
        return; 
    }

    for(i = 0; i < img->height; i++) {
        for(j = 0; j < img->width; j++) { 
            printf("{ ");
            for(k = 0; k < img->channels; k++) { 
                index = (i * img->width + j) * img->channels;
                printf("%"PRIu16" ", img->data[index + k]);
            }
            printf("},");
        }
        printf("\n");
    }
}

