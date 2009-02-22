#include <gd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
struct texthead {
    unsigned short height;
    unsigned short width;
    unsigned int format;
    unsigned int offs;
    unsigned int wrap_s;
    unsigned int wrap_t;
    unsigned int min;
    unsigned int mag;
    float lod_bias;
    unsigned char edge_lod;
    unsigned char min_lod;
    unsigned char max_lod;
    unsigned char unpacked;
};
int main(int argc, char **argv) {
    if(argc != 3) {
        printf("Usage: gentpl <input.png> <output.tpl>\n");
        exit(1);
    }
    gdImagePtr im;
    FILE *in = fopen(argv[1], "rb");
    if(!in) {
        printf("Failed to open %s\n", argv[1]); // why strerror not working?
        exit(1);
    }
    im = gdImageCreateFromPng(in);
    fclose(in);

    int width = gdImageSX(im);
    int height = gdImageSY(im);

    if(width % 4 != 0 || height % 4 != 0) {
        printf("Warning: Dimensions (%dx%d) not multiples of 4.\n", width, height);
    }
    
    FILE *out = fopen(argv[2], "wb");
    if(!out) {
        printf("Failed to open %s\n", argv[2]); // why strerror not working?
        exit(1);
    }

    char magic[] = {0x00, 0x20, 0xaf, 0x30};
    fwrite(magic, 1, 4, out);
    
    unsigned int stuff[] = {htonl(1), htonl(0xc), htonl(0x14), 0};
    fwrite(stuff, 4, 4, out);
    
    struct texthead h;
    memset(&h, 0, sizeof(h));
    
    h.format = htonl(4); // RGB565
    h.offs = htonl(0x14 + sizeof(h) + 8);
    printf("%dx%d\n", gdImageSX(im), gdImageSY(im));
    h.width = htons(gdImageSX(im));
    h.height = htons(gdImageSY(im));
    h.min = htons(1);
    h.mag = htons(1);
    
    fwrite(&h, sizeof(h), 1, out);
    
    for(int i = 0; i < 8; i++) fputc(0, out);
    
    for(int y = 0; y < height; y += 4) {
        for(int x = 0; x < width; x += 4) {
            for(int y1 = y; y1 < y + 4; y1++) {
                for(int x1 = x; x1 < x + 4; x1++) {
                    int p;
                    if(x1 >= width || y1 >= height) {
                        p = 0;
                    } else {
                        p = gdImageGetPixel(im, x1, y1);
                    }
                    unsigned char a = 254 - 2*((unsigned char) gdImageAlpha(im, p));
                    if(a == 254) a++;

                    unsigned char r = (unsigned char) gdImageRed(im, p);
                    unsigned char g = (unsigned char) gdImageGreen(im, p);
                    unsigned char b = (unsigned char) gdImageBlue(im, p);
                    unsigned short s = 0;
                    s |= (r >> 3) << 11;
                    s |= (g >> 2) << 5;
                    s |= (b >> 3);
                    fputc(s >> 8, out);
                    fputc(s & 0xff, out);
                }
            }
        
        }
    }
    
    fclose(out);
    gdImageDestroy(im);
}
