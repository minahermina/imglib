#include <unistd.h>
#include <stdio.h>

#include "src/image.h"

int main(int argc, char const *argv[]) {
    const char *out_file;
    uint16_t x,y;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <image_file>\n", argv[0]);
        return 1;
    }

    ImagePtr img = img_load(argv[1]);
    if (img == NULL) {
        fprintf(stderr, "Failed to load image: %s\n", argv[1]);
        return 1;
    }

    printf("Image loaded successfully!\n");
    printf("Width: %d, Height: %d, Channels: %d\n", img->width, img->height, img->channels);

    uint8_t pixel[4] = {0, 0, 0, 0}; // max 4 channels for each pixel
    if (img_getpx(img, 0, 0, pixel) < 0) {
        fprintf(stderr, "Failed to get pixel at (0, 0).\n");
        img_free(img);
        return 1;
    }
    printf("Pixel at (0, 0): R=%d, G=%d, B=%d\n", pixel[0], pixel[1], pixel[2]);

    // Manipulate the image: Invert the colors of each pixel
    for (y = 0; y < img->height; y++) {
        for (x = 0; x < img->width; x++) {

            if (img_getpx(img, x, y, pixel) < 0) {
                fprintf(stderr, "Failed to get pixel at (%d, %d).\n", x, y);
                img_free(img);
                return 1;
            }

            for (int i = 0; i < img->channels; i++) {
                pixel[i] = 255 - pixel[i];
            }

            // Set the modified pixel back to the image
            if (img_setpx(img, x, y, pixel) < 0) {
                fprintf(stderr, "Failed to set pixel at (%d, %d).\n", x, y);
                img_free(img);
                return 1;
            }
        }
    }

    out_file = "output_inverted.ppm";
    if (img_save(img, out_file) < 0) {
        fprintf(stderr, "Failed to save modified image to %s.\n", out_file);
        img_free(img);
        return 1;
    }
    printf("Modified image saved to %s.\n", out_file);

    // Display the modified image using an external viewer
    printf("Displaying modified image...\n");
    if (img_disp(img, "sxiv") < 0) {
        fprintf(stderr, "Failed to display image.\n");
        img_free(img);
        return 1;
    }

    // Free the image resources
    img_free(img);

    return 0;
}
