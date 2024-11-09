#include <unistd.h>
#include "image.h"

extern char etext, edata, end;

int main (int argc, char const *argv[]) {
    #define WIDTH  6 
    #define HEIGHT 6 

    uint16_t ptr[WIDTH * HEIGHT] = {
            6, 3, 1, 5, 6, 1,
            2, 5, 7, 4, 5, 3,
            5, 2, 3, 0, 3, 6,
            3, 2, 5, 4, 3, 2,
            4, 3, 2, 5, 1, 4,
            4, 3, 5, 1, 2, 5,
        };

    int x = 2, y = 0, channels = 2;
    ImagePtr img = create_img(WIDTH/channels, HEIGHT, channels);

    /* img.data = ptr ;
    img.channels = channels ;
    img.width = 2 ;
    img.height = HEIGHT; */
    setpixel(img, 0,1,(uint16_t[]){1,3});
    setpixel(img, 1,1,(uint16_t[]){3,0});
    print_img(img);


}
