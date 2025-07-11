#include <unistd.h>
#include <stdio.h>

#include "src/image.h"

int 
main(int argc, char const *argv[]) 
{
    ImgError err ;
    const char *filename;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <image_file>\n", argv[0]);
        return 1;
    }

    filename = argv[1];
    Image img = {0}, img2 = {0};

    err = img_load(&img, filename);
    if (err != IMG_OK) {
        fprintf(stderr, "Failed to load image due: %s\n", img_err2str(err));
        return 1;
    }
    
    img_cpy(&img2, &img);
    err = img_disp(&img, "sxiv");
    if(err != IMG_OK){
        fprintf(stderr, "Failed to load image due: %s\n", img_err2str(err));
        return 1;
    }

    err = img_disp(&img2, "sxiv");
    if(err != IMG_OK){
        fprintf(stderr, "Failed to load image due: %s\n", img_err2str(err));
        return 1;
    }



    Image dest; 
    err = img_resize(&dest, &img2, img2.width/6, img2.height/6);
    if(err != IMG_OK){
        fprintf(stderr, "Failed to load image due: %s\n", img_err2str(err));
        return 1;
    }

    err = img_disp(&dest, "sxiv");
    if(err != IMG_OK){
        fprintf(stderr, "Failed to load image due: %s\n", img_err2str(err));
        return 1;
    }

    err = img_save(&dest, "./tset.img");
    if(err != IMG_OK){
        fprintf(stderr, "Failed to save image due: %s\n", img_err2str(err));
        return 1;
    }

    /* printf("Image loaded successfully!\n");
    printf("Width: %d, Height: %d, Channels: %d\n", img->width, img->height, img->channels);

    uint8_t pixel[4] = {0, 0, 0, 0}; // max 4 channels for each pixel
    if (img_getpx(img, 0, 0, pixel) < 0) {
        fprintf(stderr, "Failed to get pixel at (0, 0).\n");
        img_free(img);
        return 1;
    }
    printf("Pixel at (0, 0): R=%d, G=%d, B=%d\n", pixel[0], pixel[1], pixel[2]);

    // Create a copy for comparison
    ImagePtr original_img = img_cpy(img);
    if (original_img == NULL) {
        fprintf(stderr, "Failed to create a copy of the image.\n");
        img_free(img);
        return 1;
    }

    // Demonstrate kernel operations
    printf("Applying box blur filter...\n");

    // Use the img_filter2D function with box blur kernel
    if (img_filter2D(img, KERNEL_BOX_BLUR, IMG_KERNEL_3x3, IMG_BORDER_REPLICATE) < 0) {
        fprintf(stderr, "Failed to apply box blur filter.\n");
        img_free(img);
        img_free(original_img);
        return 1;
    }

    out_file = "output_blurred.ppm";
    if (img_save(img, out_file) < 0) {
        fprintf(stderr, "Failed to save blurred image to %s.\n", out_file);
        img_free(img);
        img_free(original_img);
        return 1;
    }
    printf("Blurred image saved to %s.\n", out_file);

    // Create a sharpened version
    printf("Applying sharpen filter...\n");

    // Get the kernel directly and use img_convolve
    Kernel sharpen_kernel = img_get_kernel(KERNEL_SHARPEN, IMG_KERNEL_3x3);
    if (sharpen_kernel.data == NULL) {
        fprintf(stderr, "Failed to create sharpen kernel.\n");
        img_free(img);
        img_free(original_img);
        return 1;
    }

    // Print the kernel to see its values
    printf("Sharpen kernel:\n");
    img_print_kernel(sharpen_kernel);

    // Apply convolution with the sharpen kernel to the original image
    img_convolve(original_img, sharpen_kernel, IMG_BORDER_REPLICATE);
    img_free_kernel(sharpen_kernel);

    out_file = "output_sharpened.ppm";
    if (img_save(original_img, out_file) < 0) {
        fprintf(stderr, "Failed to save sharpened image to %s.\n", out_file);
        img_free(img);
        img_free(original_img);
        return 1;
    }
    printf("Sharpened image saved to %s.\n", out_file);

    // Create a grayscale version
    printf("Converting image to grayscale...\n");
    ImagePtr gray_img = img_rgb2gray(img);
    if (gray_img == NULL) {
        fprintf(stderr, "Failed to convert image to grayscale.\n");
        img_free(img);
        img_free(original_img);
        return 1;
    }

    out_file = "output_grayscale.pgm";
    if (img_save(gray_img, out_file) < 0) {
        fprintf(stderr, "Failed to save grayscale image to %s.\n", out_file);
        img_free(img);
        img_free(original_img);
        img_free(gray_img);
        return 1;
    }
    printf("Grayscale image saved to %s.\n", out_file);

    // Display one of the processed images
    printf("Displaying blurred image...\n");
    if (img_disp(img, "sxiv") < 0) {
        fprintf(stderr, "Failed to display image.\n");
    }

    // Free all image resources
    img_free(img);
    img_free(original_img);
    img_free(gray_img); */

    return 0;
}
