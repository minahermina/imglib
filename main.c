#include <unistd.h>
#include <stdio.h>

#include "image.h"

int main(int argc, char const *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <image_file>\n", argv[0]);
        return 1;
    }

    // Load the image from the file specified in the command line argument
    ImagePtr img = loadimg(argv[1]);
    if (img == NULL) {
        fprintf(stderr, "Failed to load image: %s\n", argv[1]);
        return 1;
    }

    printf("Image loaded successfully!\n");
    printf("Width: %d, Height: %d, Channels: %d\n", img->width, img->height, img->channels);

    uint8_t pixel[4] = {0, 0, 0, 0}; // max 4 channels for each pixel
    if (getpixel(img, 0, 0, pixel) < 0) {
        fprintf(stderr, "Failed to get pixel at (0, 0).\n");
        freeimg(img);
        return 1;
    }
    printf("Pixel at (0, 0): R=%d, G=%d, B=%d\n", pixel[0], pixel[1], pixel[2]);

    // Manipulate the image: Invert the colors of each pixel
    uint16_t x,y;
    for (y = 0; y < img->height; y++) {
        for (x = 0; x < img->width; x++) {

            if (getpixel(img, x, y, pixel) < 0) {
                fprintf(stderr, "Failed to get pixel at (%d, %d).\n", x, y);
                freeimg(img);
                return 1;
            }

            for (int i = 0; i < img->channels; i++) {
                pixel[i] = 255 - pixel[i];
            }

            // Set the modified pixel back to the image
            if (setpixel(img, x, y, pixel) < 0) {
                fprintf(stderr, "Failed to set pixel at (%d, %d).\n", x, y);
                freeimg(img);
                return 1;
            }
        }
    }

    const char *output_file = "output_inverted.ppm";
    if (saveimg(img, output_file) < 0) {
        fprintf(stderr, "Failed to save modified image to %s.\n", output_file);
        freeimg(img);
        return 1;
    }
    printf("Modified image saved to %s.\n", output_file);

    // Display the modified image using an external viewer
    printf("Displaying modified image...\n");
    if (dispimg(img, "sxiv") < 0) {
        fprintf(stderr, "Failed to display image.\n");
        freeimg(img);
        return 1;
    }

    // Free the image resources
    freeimg(img);

    return 0;
}
