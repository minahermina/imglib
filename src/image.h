/*
A minimal Image Processing in pure C library
Copyright (C) 2025  Mina Albert Saeed <mina.albert.saeed@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef IMAGE_LIB
#define IMAGE_LIB
#include <stdint.h>

#define IMG_ERROR_MAX_STRING_SIZE 64

#define img_err2str(status) \
    img_strerror((char[IMG_ERROR_MAX_STRING_SIZE]){0}, IMG_ERROR_MAX_STRING_SIZE, status)

typedef enum {
    IMG_UNKNOWN = -1,
    IMG_PPM_BIN = 0x5036,   // P6
    IMG_PPM_ASCII = 0x5033, // P3
    IMG_PGM_BIN = 0x5035,   // P5
    IMG_PGM_ASCII = 0x5032, // P2
} ImgType;

typedef enum {
    IMG_KERNEL_3x3 = 3,
    IMG_KERNEL_5x5 = 5,
    IMG_KERNEL_7x7 = 7,
    IMG_KERNEL_11x11 = 7,
} KernelSize;

typedef enum {
    IMG_KERNEL_IDENTITY,
    IMG_KERNEL_BOX_BLUR,
    IMG_KERNEL_GAUSSIAN_BLUR,
    IMG_KERNEL_SHARPEN,
    IMG_KERNEL_SOBEL_X,
    IMG_KERNEL_SOBEL_Y,
    IMG_KERNEL_LAPLACIAN,
} KernelType;

typedef enum {
    IMG_OK                      =  0,   /* No error, operation successful                */
    IMG_ERR_FILE_NOT_FOUND      = -1,   /* The specified image file was not found        */
    IMG_ERR_FILE_READ           = -2,   /* Error reading the image file                  */
    IMG_ERR_FILE_WRITE          = -3,   /* Error writing to the image file               */
    IMG_ERR_FILE_CREATE         = -4,
    IMG_ERR_UNSUPPORTED_FORMAT  = -5,   /* The image format is not supported or invalid  */
    IMG_ERR_MEMORY              = -6,   /* Memory allocation failed                      */
    IMG_ERR_INVALID_PARAMETERS  = -7,
    IMG_ERR_INVALID_DIMENSIONS  = -8,   /* Invalid dimensions passed                      */
    /* maybe used ? idk */
    IMG_ERR_COLOR_SPACE         = -9,   /* Unsupported or invalid color space            */
    IMG_ERR_CORRUPT_DATA        = -10,   /* The image data is corrupted                   */
    IMG_ERR_UNKNOWN             = -11    /* Unknown error                                 */
} ImgError;

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
    ImgError status;
} Image;

typedef struct {
    size_t size;
    float *data;
} Kernel;

typedef enum {
    IMG_BORDER_ZERO_PADDING,
    IMG_BORDER_REPLICATE
} BorderMode;


ImgError img_init(Image *img, uint16_t width, uint16_t height, uint8_t channels); /**/
Image img_create(uint16_t width, uint16_t height, uint8_t channels);/*    */
ImgError img_load(Image *img, const char* file); //
ImgError img_loadpnm(Image *img, const char* file, ImgType type);//
ImgError img_getpx(Image *img, uint16_t x, uint16_t y, uint8_t *pixel); //
ImgError img_setpx(Image *img, uint16_t x, uint16_t y, uint8_t *pixel); //
ImgError img_savepnm(Image *img, const char *file); //
ImgError img_save(Image *img, const char *file); //
ImgError img_cpy(Image *dest, Image *src);
void img_free(Image img);
void img_print(Image *img);
ImgError img_disp(Image *img, const char* custom_viewer);
const char *img_strerror(char *buf, size_t sz , ImgError err);

/*Image Processing Functions*/

/* ----------- Kernel stuff----------- */
Kernel img_get_kernel(KernelType type, KernelSize size);
int8_t img_filter2D(Image *img, KernelType type, KernelSize size, BorderMode border_mode);
void img_print_kernel(Kernel kernel);
void img_free_kernel(Kernel kernel);
/* ------------------------------------*/
void img_convolve(Image *img, Kernel kernel, BorderMode border_mode);
ImgError img_rgb2gray(Image *dest, Image *img);
ImgError img_resize(Image *dest, Image *src, uint16_t new_width, uint16_t new_height); //
ImgError img_add(Image *dest, Image *img1, Image *img2); //
ImgError img_subtract(Image *dest, Image *img1, Image *img2); //

#endif
