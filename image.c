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

/* Returns NULL in case of error*/
ImagePtr 
createimg(uint16_t width, uint16_t height, uint8_t channels)
{
    ImagePtr img;
    img = (ImagePtr) calloc(1, sizeof(image));
    if(img == NULL) {
        fprintf(stderr, "Buy more RAM please");
        return NULL;
    }

    img->data= (uint16_t*) calloc(width * height * channels, sizeof(uint16_t));
    if(img->data == NULL) {
        fprintf(stderr, "Buy more RAM please");
        return NULL;
    }

    img->width    = width;
    img->height   = height;
    img->channels = channels;


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
    
    channels = 3; /*PPM*/
    img = createimg(w, h, channels);

    i = 0;
    /* TODO : read the PPM image*/   
    while ((fread(buf, 1, 3, imagefile)) == 3 && i < w * h) {
        for(uint8_t channel = 0; channel < channels; channel++){
            pixel_values[channel] = buf[channel];
        }
        uint16_t x = i % w;
        uint16_t y = i / w;
        // printf("%hu %hu \n", x, y);

        setpixel(img, x, y, pixel_values);
        i++;
    }

    fclose(imagefile);
    return img;
}
uint8_t saveppm(const char *file, ImagePtr img){
    uint8_t px[3] = {};
    if(img == NULL){
        fprintf(stderr, "img is NULL!");
        return -1;
    }

    FILE * imgfile = fopen(file, "wb");
    if(imgfile == NULL){
        fprintf(stderr, "failed to open file %s", file);
        return -1;
    }
    fprintf(imgfile, "P6\n%hu %hu\n255\n", img->width, img->height);
    for (uint32_t i = 0; i < img->width * img->height; i+=3) {
        for (uint8_t channel = 0; channel < img->channels; channel++) {
            px[channel] =  img->data[i + channel];
        }
        printf("pixel: %hhu %hhu %hhu\n", px[0], px[1], px[2]);
        fwrite(&px, 1, 3, imgfile);
    }
    return 0;

}

int8_t 
free_img(ImagePtr img)
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

    size_t index ;
    for(size_t i = 0; i < img->height; i++) {
        for(size_t j = 0; j < img->width; j++) { 
            printf("{ ");
            for(size_t k = 0; k < img->channels; k++) { 
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
    ImagePtr img;
    switch(imgtype(file)){
        case PPM_MAGIC_BIN:
        case PPM_MAGIC_ASCII:
            img = loadppm(file);
        default:
            break;
    }

    // return NULL;
}

