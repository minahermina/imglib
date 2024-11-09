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

int8_t getpixel(ImagePtr img , uint16_t  x, uint16_t  y, uint16_t *pixel_values);
ImagePtr loadimg(const char *file, ImagePtr img);
ImagePtr create_img(uint16_t weight, uint16_t height ,uint8_t channels);
ImagePtr loadimg_ppm(uint16_t width, uint16_t h,uint8_t channels);
int8_t setpixel(ImagePtr img, uint16_t  x, uint16_t  y, uint16_t *pixel_values);
void print_img(ImagePtr img);

#endif
