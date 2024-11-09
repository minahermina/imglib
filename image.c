#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include "image.h"


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
        return 0;
    }

    size_t index = (x * img->width + y) * img->channels;
    for(uint8_t i = 0; i < img->channels; i++ ) {
         img->data[index + i] = pixel_values[i];
    }
    return 1;
}

void 
print_img(ImagePtr img)
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
create_img(uint16_t width, uint16_t height, uint8_t channels)
{
    printf(": %d\n", width);
    printf(": %d\n", height);
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

ImagePtr
loadimg(const char* file, ImagePtr img)
{


}

