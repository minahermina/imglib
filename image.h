#ifndef IMAGE_LIB
#define IMAGE_LIB
#include <stdint.h>

#define PPM_MAGIC_ASCII 0x50330A /* P3 */
#define PPM_MAGIC_BIN   0x50360A /* P6 */
#define PGM_MAGIC_ASCII 0x50320A /* P2 */
#define PGM_MAGIC_BIN   0x50330A /* P5 */

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
uint8_t saveppm(const char *file, ImagePtr img);
void freeimg(ImagePtr img);
int8_t getpixel(ImagePtr img , uint16_t  x, uint16_t  y, uint16_t *pixel_values);
int8_t setpixel(ImagePtr img, uint16_t  x, uint16_t  y, uint16_t *pixel_values);
void printimg(ImagePtr img);

#endif
