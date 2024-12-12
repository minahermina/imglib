#ifndef IMAGE_LIB
#define IMAGE_LIB
#include <stdint.h>

typedef enum {
    IMG_UNKNOWN = -1,
    IMG_PPM_BIN = 0x5036,    // P6
    IMG_PPM_ASCII = 0x5033,  // P3
    IMG_PGM_BIN = 0x5035,    // P5
    IMG_PGM_ASCII = 0x5032,  // P2
    IMG_PNG = 0x8950,        // \x89P
    IMG_JPG = 0xFFD8,        // \xFF\xD8
    IMG_BMP = 0x424D,        // BM
    IMG_GIF = 0x4749,        // GI
    IMG_TIFF = 0x4949        // II
} ImgType;

typedef struct {
    uint8_t *data;
    uint32_t stride;
    uint16_t width;
    uint16_t height;
    uint8_t  channels;
    ImgType type;
} image;

typedef image *ImagePtr;

ImagePtr createimg(uint16_t width, uint16_t height ,uint8_t channels);
ImagePtr loadimg(const char *file);
int64_t imgtype(const char *file);
ImagePtr loadppm(const char* file);
uint8_t saveppm(const char *file, ImagePtr img);
void freeimg(ImagePtr img);
int8_t getpixel(ImagePtr img , uint16_t  x, uint16_t  y, uint16_t *pixel_values);
int8_t setpixel(ImagePtr img, uint16_t  x, uint16_t  y, uint16_t *pixel_values);
void printimg(ImagePtr img);

#endif
