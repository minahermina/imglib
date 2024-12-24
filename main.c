#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "image.h"

int main (int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <image_file>\n", argv[0]);
        return 1;
    }

    ImagePtr img = loadimg(argv[1]);
    if(img != NULL){
        dispimg(img, "sxiv");
    }
    return 0;
}
