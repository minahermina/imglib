#include <unistd.h>
#include <fcntl.h>

#include "image.h"

#define WIDTH  6 
#define HEIGHT 6 

int main (int argc, char const *argv[]) {
    int x = 2, y = 0, channels = 2;
    // ImagePtr img = create_img(WIDTH, HEIGHT, channels);
    ImagePtr img = loadimg("./images/stop_2.ppm");
    // print_img(img);


}
