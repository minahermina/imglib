#ifndef IMAGE_LIB
#define IMAGE_LIB
#include <stdint.h>

#define PPM_MAGIC1 0x50360A
#define PPM_MAGIC2 0x50330A
#define PNG_MAGIC  0x89504e47

typedef struct {
    uint16_t *data;
    uint16_t  width;
    uint16_t  height;
    uint8_t  channels;
} image;

typedef image *ImagePtr;

int8_t getpixel(ImagePtr img , uint16_t  x, uint16_t  y, uint16_t *pixel_values);
ImagePtr loadimg(const char *file);
int64_t imgtype(const char *file);
ImagePtr create_img(uint16_t width, uint16_t height ,uint8_t channels);
int8_t free_img(ImagePtr img);
ImagePtr loadimgPPM(const char* file);
int8_t setpixel(ImagePtr img, uint16_t  x, uint16_t  y, uint16_t *pixel_values);
void print_img(ImagePtr img);

#endif
