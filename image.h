#ifndef IMAGE_LIB
#define IMAGE_LIB
#include <stdint.h>


typedef struct {
    uint16_t *data;
    uint16_t  width;
    uint16_t  height;
    uint8_t  channels;
} image;

typedef image *ImagePtr;

ImagePtr createimg(uint16_t width, uint16_t height ,uint8_t channels);
ImagePtr loadimg(const char *file);
int64_t imgtype(const char *file);
ImagePtr loadppm(const char* file);
int8_t freeimg(ImagePtr img);
int8_t getpixel(ImagePtr img , uint16_t  x, uint16_t  y, uint16_t *pixel_values);
int8_t setpixel(ImagePtr img, uint16_t  x, uint16_t  y, uint16_t *pixel_values);
void printimg(ImagePtr img);

#endif
