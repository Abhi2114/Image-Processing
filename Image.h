// Header file that defines a class for the Image and
// declarations of all the different operations that you could perform
// on the image

#ifndef IMAGE_H
#define IMAGE_H

#include "pixel.h"
#include <vector>

class Image {
        // model standard image attributes like specs(dimensions, no of channels),
        // the actual image data and the matrix like interface which holds pointers
        // to each and every scanline of the raster image
private:
        int width, height, channels;
        unsigned char *pixmap;
        unsigned char **matrix;  // access in true matrix style
public:
        Image(int width, int height, int channels);

        // call to clean up
        void destroy() {
            delete[] matrix;
            delete[] pixmap;
        }
        void copyImage(const unsigned char *pixmap_);
        // define some getters
        int getWidth()       { return width; }
        int getHeight()      { return height; }
        unsigned char* getPixmap() { return pixmap; }

        // routines to get and set pixel values at the given pixel location(x, y)
        pixel getpixel(int x, int y) {
            unsigned char red = matrix[x][4*y];
            unsigned char green = matrix[x][4*y + 1];
            unsigned char blue = matrix[x][4*y + 2];
            unsigned char alpha = matrix[x][4*y + 3];

            return pixel(red, green, blue, alpha);
        }

        void setpixel(int x, int y, pixel pix) {
            matrix[x][4*y] = pix.r;
            matrix[x][4*y + 1] = pix.g;
            matrix[x][4*y + 2] = pix.b;
            matrix[x][4*y + 3] = pix.a;
        }

        void inverse();

        // reverse the image for display purposes, returns a new image
        Image* flip();

        // greyscale operations
        void greyscaleRed();
        void greyscaleGreen();
        void greyscaleBlue();

        void toBitmap();
        void reducePalette(std::vector<pixel> &palette);

        // using median cut
        void getReducedPalette(std::vector<pixel> &palette);  // the results will be populated
        // into the palette

        // floyd steinberg dithering
        void floydSteinberg(std::vector<pixel> &palette);
};

#endif
