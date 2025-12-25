#include <stdio.h>
#include <stdlib.h>
#include "src/image.h"

// A simple macro to check for errors and exit if one occurs.
#define CHECK_STATUS(err) \
    if (err != IMG_OK) { \
        fprintf(stderr, "Error: %s\n", img_err2str(err)); \
        return 1; \
    }

int main(int argc, char *argv[]) {
    Arena arena = {0};
    arena_init(&arena, 10 * 1024 * 1024);

    Image img = {0},
          gray_img = {0},
          filtered_img = {0},
          resized_img = {0};

    ImgError err;

    // --- 1. Load an image ---
    const char *input_path = "images/sample_1920×1280.ppm";
    err = img_load(&img, input_path, &arena);
    CHECK_STATUS(err);

    // --- 2. Convert to Grayscale ---
    if (img.channels >= 3) {
        err = img_rgb2gray(&gray_img, &img);
        CHECK_STATUS(err);

        const char *gray_path = "grayscale_output.pgm";
        err = img_save(&gray_img, gray_path);
        CHECK_STATUS(err);
    }

    // --- 3. Apply a Sharpen Filter ---
    err = img_filter2D(&filtered_img,
                       &img,
                       IMG_KERNEL_SHARPEN,
                       IMG_KERNEL_3x3,
                       IMG_BORDER_REPLICATE);
    CHECK_STATUS(err);

    const char *filtered_path = "sharpened_output.ppm";
    err = img_save(&filtered_img, filtered_path);
    CHECK_STATUS(err);

    // --- 4. Resize the Image ---
    uint16_t new_width = 320;
    uint16_t new_height = 213;
    err = img_resize(&resized_img, &img, new_width, new_height);
    CHECK_STATUS(err);

    const char *resized_path = "resized_output.ppm";
    err = img_save(&resized_img, resized_path);
    CHECK_STATUS(err);

    // --- 5. Display the original image ---
    // Note: This requires an image viewer like 'eog', and 'feh' to be installed.
    img_disp(&img, "sxiv");
    arena_destroy(&arena);


    return 0;
}
