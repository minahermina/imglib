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
#include <stdlib.h>

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
    KERNEL_IDENTITY,
    KERNEL_BOX_BLUR,
    KERNEL_GAUSSIAN_BLUR,
    KERNEL_SHARPEN,
    KERNEL_SOBEL_X,
    KERNEL_SOBEL_Y,
    KERNEL_LAPLACIAN,
} KernelType;

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
} Image;

typedef struct {
    size_t size;
    float *data;
} Kernel;

typedef enum {
    IMG_BORDER_ZERO_PADDING,
    IMG_BORDER_REPLICATE
} BorderMode;

typedef Image *ImagePtr;

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

/* ----------- Kernel stuff----------- */
Kernel img_get_kernel(KernelType type, KernelSize size);
int8_t img_filter2D(ImagePtr img, KernelType type, KernelSize size, BorderMode border_mode);
void img_print_kernel(Kernel kernel);
void img_free_kernel(Kernel kernel);
/* ------------------------------------*/
void img_convolve(ImagePtr img, Kernel kernel, BorderMode border_mode);
ImagePtr img_rgb2gray(ImagePtr img);
ImagePtr img_resize(ImagePtr src, uint16_t new_width, uint16_t new_height);
ImagePtr img_add(ImagePtr img1, ImagePtr img2);

#endif
