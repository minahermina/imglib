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

ImagePtr createimg(uint16_t width, uint16_t height, uint8_t channels);
void freeimg(ImagePtr img);
ImgType imgtype(const char *file);
ImagePtr loadimg(const char* file);
ImagePtr loadppm(const char* file);
uint8_t saveppm( ImagePtr img, const char *file);
uint8_t savepgm( ImagePtr img, const char *file);
uint8_t saveimg( ImagePtr img, const char *file);
int8_t getpixel(ImagePtr img, uint16_t x, uint16_t y, uint8_t *pixel);
int8_t setpixel(ImagePtr img, uint16_t x, uint16_t y, uint8_t *pixel);
void printimg(ImagePtr img);
int8_t dispimg(ImagePtr img, const char* custom_viewer);
/*Image manipulation Functions*/
ImagePtr rgb2gray(ImagePtr img);


#endif
