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
int64_t 
imgtype(const char *file)
{
    #define BUFSIZE 3
    uint8_t buf[BUFSIZE];
    int fd = open(file, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error opening file %s \n%s", file , strerror(errno));
        return -1;
    }

    /* TODO : find approach to read the magic number */
    ssize_t bytesread = read(fd, buf, BUFSIZE);
    if (bytesread < 0){
        fprintf(stderr, "Error reading from file%s \n%s", file , strerror(errno));
        return -1;
    }

    close(fd);
    uint64_t magicnum = (buf[0] << 16) | (buf[1] << 8) | buf[2];
    return magicnum;
}

/* TODO : handle other PPM formats (PGM)*/
ImagePtr 
loadppm(const char* file)
{
    ImagePtr img;
    FILE* imagefile;
    uint16_t pixel_values[3], w, h;
    uint32_t i;
    uint8_t channels;
    char line[MAX_LINE_LENGTH];
    unsigned char buf[3];

    imagefile = fopen(file, "rb" );
    if (imagefile == NULL)
        return NULL;

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


/* Return -1 in case of out of boundries*/
int8_t 
getpixel(ImagePtr img, uint16_t  x, uint16_t  y, uint16_t *pixel_values)
{
    if (x >= img->height || y >= img->width) {
        return -1;
    }

    size_t index = (x * img->width + y) * img->channels;
    for(uint8_t i = 0; i < img->channels; i++ ) {
        pixel_values[i] = img->data[index + i];
    }

    return 1;
}

/* Return -1 in case of out of boundries*/
int8_t 
setpixel(ImagePtr img, uint16_t  x, uint16_t  y, uint16_t *pixel_values)
{
    if (x >= img->height || y >= img->width) {
        return -1;
    }

    size_t index = (x * img->width + y) * img->channels;
    for(uint8_t i = 0; i < img->channels; i++ ) {
         img->data[index + i] = pixel_values[i];
    }
    return 1;
}

void 
printimg(ImagePtr img)
{
    if (img == NULL) {
        fprintf(stderr, "img is NULL!");
        return; 
    }

    uint16_t index ;
    for(uint16_t i = 0; i < img->height; i++) {
        for(uint16_t j = 0; j < img->width; j++) { 
            printf("{ ");
            for(uint8_t k = 0; k < img->channels; k++) { 
                index = (i * img->width + j) * img->channels;
                printf("%"PRIu16" ", img->data[index + k]);
            }
            printf("},");
        }
        printf("\n");
    }
}




ImagePtr
loadimg(const char* file)
{
    ImagePtr img = NULL;
    switch(imgtype(file)){
        case PPM_MAGIC_BIN: /* FALLTHROUGH */
        case PPM_MAGIC_ASCII:
            img = loadppm(file);
            break;
        default:
             fprintf(stderr, "This image format is not supported yet!!\n");
            break;
    }

    return img;
}

