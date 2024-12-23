#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "image.h"

#define MAX_LINE_LENGTH 200

#define IMG_PIXEL_PTR(img, x, y) ((uint8_t*)((img)->data + (y) * (img)->stride + (x) * (img)->channels))

static inline uint32_t 
calc_stride(uint16_t width, uint8_t channels) {
    return (((uint32_t) width * (uint32_t)channels + 15) & ~(uint32_t)15);
}

ImagePtr 
createimg(uint16_t width, uint16_t height, uint8_t channels)
{
    ImagePtr img;
    img = (ImagePtr) calloc(1, sizeof(image));
    if(img == NULL) {
        fprintf(stderr, "Buy More RAM LOL!\n");
        return NULL;
    }

    img->stride = calc_stride(width, channels);
    img->data = (uint8_t*) malloc(height * img->stride);
    if(img->data == NULL) {
        fprintf(stderr, "Buy more RAM LOL!\n");
        free(img);
        return NULL;
    }

    memset(img->data, 0, height * img->stride);

    img->width = width;
    img->height = height;
    img->channels = channels;
    img->type = -1;

    return img;
}


/* TODO : Handle other image formats: PNG, JPG */
ImgType 
imgtype(const char *file)
{
      int fd;
    ssize_t bytesread;
    unsigned char magic[8];  // Buffer for magic numbers
    uint16_t magic_num;
    
    fd = open(file, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error opening file %s\n%s", file, strerror(errno));
        return IMG_UNKNOWN;
    }

    bytesread = read(fd, magic, sizeof(magic));
    close(fd);
    
    if (bytesread < 2) {
        return IMG_UNKNOWN;
    }

    magic_num = (magic[0] << 8) | magic[1];
    
    switch(magic_num) {
        case 0x5036: return IMG_PPM_BIN;
        case 0x5033: return IMG_PPM_ASCII;
        case 0x5035: return IMG_PGM_BIN;
        case 0x5032: return IMG_PGM_ASCII;
        case 0x8950: return IMG_PNG;
        case 0xFFD8: return IMG_JPG;
        case 0x424D: return IMG_BMP;
        case 0x4749: return IMG_GIF;
        case 0x4949:
        case 0x4D4D: return IMG_TIFF;
        default: return IMG_UNKNOWN;
    }
}

int8_t 
addpixel(ImagePtr img, const uint8_t *pixel, uint32_t *current_pos)
{
    uint8_t *p, i;
    uint32_t x, y;

    if (img == NULL || pixel == NULL || current_pos == NULL) {
        fprintf(stderr, "Error: Invalid input (img, pixel, or position is NULL).\n");
        return -1;
    }

    if (*current_pos >= (uint32_t)(img->width * img->height)) {
        fprintf(stderr, "Error: Exceeded image capacity.\n");
        return -1;
    }

    x = *current_pos % img->width;
    y = *current_pos / img->width;
    p = IMG_PIXEL_PTR(img, x, y);

    for (i = 0; i < img->channels; i++) {
        p[i] = pixel[i];
    }

    (*current_pos)++;

    return 1;
}

/* TODO : handle other PPM formats (PGM)*/
ImagePtr 
loadppm(const char* file)
{
    ImagePtr img;
    ssize_t imgfile;
    FILE* tmp_file;
    uint8_t pixel[3], *row, channels, ch;
    uint16_t w, h;
    uint32_t j, i, curr_pos, bytesread;
    char line[MAX_LINE_LENGTH], chunk[CHUNK_SIZE];

    tmp_file = fopen(file, "rb");
    if (tmp_file == NULL)
        return NULL;

    /* Read header */
    fgets(line, sizeof(line), tmp_file);
    fgets(line, sizeof(line), tmp_file);
    sscanf(line, "%hu %hu", &w, &h);
    fgets(line, sizeof(line), tmp_file);

    long pos = ftell(tmp_file);
    fclose(tmp_file);

    imgfile = open(file, O_RDONLY);
    if (imgfile < 0)
        return NULL;

    lseek(imgfile, pos, SEEK_SET);

    channels = 3;
    img = createimg(w, h, channels);
    if (img == NULL) {
        close(imgfile);
        return NULL;
    }

    bytesread = 0, curr_pos = 0;

    while((bytesread = read(imgfile, chunk, sizeof(chunk))) > 0){
        for(i = 0; i < bytesread; i += channels) {
            for(ch = 0; ch < channels; ch++) {
                pixel[ch] = chunk[i + ch];
            }
            if(addpixel(img, pixel, &curr_pos) < 0) {
                close(imgfile);
                freeimg(img);
                return NULL;
            }
        }
    }

    close(imgfile);
    return img;
}

uint8_t
saveppm(ImagePtr img, const char *file)
{
    FILE *fp;
    uint32_t x, y;
    uint8_t *row, *pixel;

    if (img == NULL) {
        fprintf(stderr, "img is NULL!");
        return -1;
    }

    fp = fopen(file, "wb");
    if (!fp)
        return -1;

    // Write header
    fprintf(fp, "P6\n%d %d\n255\n", img->width, img->height);

    for(y = 0; y < img->height; y++) {
        row = &img->data[y * img->stride];
        for(x = 0; x < img->width; x++) {
            pixel = &row[x * img->channels];
            if (fwrite(pixel, 1, img->channels, fp) != img->channels) {
                fclose(fp);
                return 0;
            }
        }
    }
    fclose(fp);
    return 1;
}


uint8_t 
saveimg( ImagePtr img, const char *file)
{
    if(img == NULL || file == NULL || strlen(file) < 1) 
        return -1;

    switch (img->type) {
        case IMG_PPM_BIN:
        case IMG_PPM_ASCII:
            saveppm(img , file);
            break;
        default:
            return -1;
    }
    return 1;
}


void 
freeimg(ImagePtr img)
{
    free(img->data);
    free(img);
}


int8_t 
getpixel(ImagePtr img, uint16_t x, uint16_t y, uint8_t *pixel)
{
    uint8_t *p, i;
    if (x >= img->width || y >= img->height) {
        return -1;
    }

    p = IMG_PIXEL_PTR(img, x, y);
    for(i = 0; i < img->channels; i++) {
        pixel[i] = p[i];
    }

    return 1;
}

int8_t
setpixel(ImagePtr img, uint16_t x, uint16_t y, uint8_t *pixel)
{
    uint8_t *p, i;

    if (x >= img->width || y >= img->height) {
        return -1;
    }

    p = IMG_PIXEL_PTR(img, x, y);

    for(i = 0; i < img->channels; i++) {
        p[i] = pixel[i];
    }

    return 1;
}

ImagePtr
loadimg(const char* file)
{
    if(strlen(file) < 1 ) 
        return NULL;

    ImagePtr img = NULL;
    int64_t type = imgtype(file);

    switch(type) {
        case IMG_PPM_BIN:
        case IMG_PPM_ASCII:
            img = loadppm(file);
            if (img){
                img->type = type;
            }else{
                fprintf(stderr, "Error loading image: %s\n", file);
                return NULL;
            }
            break;
        case IMG_PGM_BIN:
        case IMG_PGM_ASCII:
            // TODO: implement PGM loading
            break;
        default:
            fprintf(stderr, "Unsupported or invalid image format\n");
    }
    
    return img;
}
void 
printimg(ImagePtr img)
{
    uint16_t i, j, k;
    uint8_t pixel[] = {0, 0, 0, 0};
    if (img == NULL) {
        fprintf(stderr, "img is NULL!");
        return;
    }

    for(i = 0; i < img->height; i++) {
        for(j = 0; j < img->width; j++) {
            printf("{ ");
            getpixel(img, j, i, pixel);
            for(k = 0; k < img->channels; k++) 
                printf("%hu ",pixel[k]);
            printf("},");
        }
        printf("\n");
    }
}

int8_t
dispimg(ImagePtr img, const char* imgviewer)
{
    char template[] = "/tmp/img_XXXXXX";
    char CMD[0xFFF];
    int fd;


    printf("Using viewer command: '%s'\n", imgviewer);

    fd = mkstemp(template);
    if(fd == -1) {
        fprintf(stderr, "Error creating temporary file\n");
        return -1;
    }

    FILE* file = fdopen(fd, "w+");
    if(file == NULL) {
        fprintf(stderr, "Error opening temporary image file\n");
        close(fd);
        unlink(template);
        return -1;
    }

    saveimg(img, template);
    sprintf(CMD, "%s %s", imgviewer, template);
    printf("Executing command: '%s'\n", CMD);
    system(CMD);

    fclose(file);
    unlink(template);

    return 0;
}
