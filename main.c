#include <unistd.h>
#include <fcntl.h>

#include "image.h"

int main (int argc, char const *argv[]) {
    ImagePtr img = loadimg(argv[1]);
    if (saveppm("./test.ppm", img)){
    }
}
