#ifndef IMAGE_LIB
#define IMAGE_LIB
#include <stdint.h>

typedef enum {
    IMG_UNKNOWN = -1,
    IMG_PPM_BIN = 0x5036,    // P6
    IMG_PPM_ASCII = 0x5033,  // P3
    IMG_PGM_BIN = 0x5035,    // P5
    IMG_PGM_ASCII = 0x5032,  // P2
} ImgType;

typedef struct {
    uint8_t *data;
    /* 
    - read this https://medium.com/@oleg.shipitko/what-does-stride-mean-in-image-processing-bba158a72bcd
    - read about memory access patterns
    */
    uint32_t stride;
    uint16_t width;
    uint16_t height;
    uint8_t  channels;
    ImgType type;
} image;

typedef image *ImagePtr;

ImagePtr img_create(uint16_t width, uint16_t height, uint8_t channels);
ImagePtr img_load(const char* file);
ImgType img_type(const char *file);
ImagePtr img_loadpnm(const char* file, ImgType type);
int8_t img_getpx(ImagePtr img, uint16_t x, uint16_t y, uint8_t *pixel);
int8_t img_setpx(ImagePtr img, uint16_t x, uint16_t y, uint8_t *pixel);
uint8_t img_savepnm(ImagePtr img, const char *file);
uint8_t img_save(ImagePtr img, const char *file);
ImagePtr img_cpy(ImagePtr src);
void img_free(ImagePtr img);
void img_print(ImagePtr img);
int8_t img_disp(ImagePtr img, const char* custom_viewer);

/*Image Processing Functions*/
ImagePtr img_rgb2gray(ImagePtr img);
ImagePtr img_resize(ImagePtr src, uint16_t new_width, uint16_t new_height);
ImagePtr img_add(ImagePtr img1, ImagePtr img2);


#endif
