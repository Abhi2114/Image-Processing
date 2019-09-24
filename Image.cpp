#include "Image.h"
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <unordered_set>
#include <cinttypes>
#include <limits>

typedef unsigned char uchar;
typedef std::int16_t int16;

Image::Image(int width, int height, int channels) :
width(width), height(height), channels(channels)
{
    int numbytes = 4 * width * height;  // always use 4 channels
    // allocate space for the pixmap
    pixmap = new unsigned char[numbytes];

    // setup the matrix access as well
    matrix = new unsigned char *[height];
    // set all the pointers appropriately
    matrix[0] = pixmap;
    for (int i = 1; i < height; ++i)
        matrix[i] = matrix[i - 1] + 4 * width;
}

// convert the input image to RGBA format if required
void Image::copyImage(const unsigned char *pixmap_) {
    // get the number of bytes to copy
    int numbytes = channels * width * height;

    if (channels == 1) {
        // greyscale image
        for (int i = 0, j = 0; i < numbytes; ++i, j += 4) {
            pixmap[j] = pixmap_[i];
            pixmap[j+1] = pixmap_[i];
            pixmap[j+2] = pixmap_[i];
            pixmap[j+3] = 255;
        }
    }
    else if (channels == 3) {
        // RGB image
        int alphas = 0;  // the number of alphas we have seen till now
        for (int i = 0; i < numbytes; ++i) {
            if ((i + alphas) % 4 == 3) {
                // we've seen an alpha
                pixmap[i + alphas] = 255;
                alphas++;
            }
            pixmap[i + alphas] = pixmap_[i];
        }
    }
    else
        memcpy(pixmap, pixmap_, numbytes);  // vanilla RGBA image, no need to do anything
}

/*
  the following 3 procedures will perform the greyscale operation
  for the 3 colors respectively.

  for eg. greyscaleRed() will copy the red value of each pixel into the other
  2 channels(blue and green) which will end up making all the red pixels white
  and all the non red pixels black(sort of).

  similarly, we can do the same thing for the green and blue color channels as well
*/

void Image::greyscaleRed() {

    // set all the b and g to red
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            pixel pix = getpixel(h, w);
            pix.b = pix.g = pix.r;
            setpixel(h, w, pix);
        }
    }
}

void Image::greyscaleGreen() {

    // set all the r and b to green
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            pixel pix = getpixel(h, w);
            pix.r = pix.b = pix.g;
            setpixel(h, w, pix);
        }
    }
}

void Image::greyscaleBlue() {

    // set the r and g to blue
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            pixel pix = getpixel(h, w);
            pix.r = pix.g = pix.b;
            setpixel(h, w, pix);
        }
    }
}

// flip the image upside down for displaying
Image* Image::flip() {

    // flip the image for displaying
    Image *reversed = new Image(width, height, channels);

    // copy the image row by row, from bottom to top, which ends up
    // flipping it
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            pixel pix = getpixel(height - h - 1, w);
            reversed->setpixel(h, w, pix);
        }
    }

    return reversed;
}

/*
  the classic invert colors operation
  implemented in exactly the same way as was given in the first quiz
*/
void Image::inverse() {

    // standard inversion operation
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            pixel pix = getpixel(h, w);
            pix.r = 255 - pix.r;
            pix.g = 255 - pix.g;
            pix.b = 255 - pix.b;
            setpixel(h, w, pix);
        }
    }
}

// dithering baby, will work only for greyscale images though
void Image::toBitmap() {

  for (int h = 0; h < height; ++h) {

    int left_error = 0;  // error is 0 at the start of every scanline

    for (int w = 0; w < width; ++w) {
      // get the current pixel
      pixel pix = getpixel(h, w);

      int intensity = pix.r;
      intensity += left_error;
      left_error = intensity;

      if (255 - intensity < intensity) {
        // closer to white
        left_error = intensity - 255;
        intensity = 255;
      }
      else
        intensity = 0;

      pix.r = pix.g = pix.b = intensity;
      setpixel(h, w, pix);
    }

  }

}

// find the closest color in the palette to the given color
int findClosestPaletteColor(pixel &color,
                            std::vector<pixel> &palette) {

    float smallest = std::numeric_limits<float>::max();
    int palette_index = -1;  // index of the closest color in the palette

    // get the color closest to the current pixel in the palette
    for (int i = 0; i < palette.size(); ++i) {
      pixel palette_color = palette[i];
      pixel difference = color - palette_color;

      // sum the squares of the differences
      float diff =  (float)(difference.r * difference.r) +
                    (float)(difference.g * difference.g) +
                    (float)(difference.b * difference.b);
      // update closest color index
      if (diff < smallest) {
        smallest = diff;
        palette_index = i;
      }
    }

    return palette_index;
}

int16 byteCap(int16 num) {
  if (num > 255) return 255;
  if (num < 0) return 0;

  return num;
}

// floyd-steinberg in action ladies
void Image::floydSteinberg(std::vector<pixel> &palette) {

  for (int h = 0; h < height-1; ++h) {
    for (int w = 1; w < width-1; ++w) {

      // get the current pixel value from the temp buffer
      pixel oldpixel = getpixel(h, w);

      pixel newpixel = palette[findClosestPaletteColor(oldpixel, palette)];

      setpixel(h, w, newpixel);

      int16 qer = oldpixel.r - newpixel.r;
      int16 qeg = oldpixel.g - newpixel.g;
      int16 qeb = oldpixel.b - newpixel.b;

      pixel neighbor = getpixel(h, w + 1);
      neighbor.r = byteCap(neighbor.r + qer * 7/16);
      neighbor.g = byteCap(neighbor.g + qeg * 7/16);
      neighbor.b = byteCap(neighbor.b + qeb * 7/16);
      setpixel(h, w + 1, neighbor);

      neighbor = getpixel(h + 1, w - 1);
      neighbor.r = byteCap(neighbor.r + qer * 3/16);
      neighbor.g = byteCap(neighbor.g + qeg * 3/16);
      neighbor.b = byteCap(neighbor.b + qeb * 3/16);
      setpixel(h + 1, w - 1, neighbor);

      neighbor = getpixel(h + 1, w);
      neighbor.r = byteCap(neighbor.r + qer * 5/16);
      neighbor.g = byteCap(neighbor.g + qeg * 5/16);
      neighbor.b = byteCap(neighbor.b + qeb * 5/16);
      setpixel(h + 1, w, neighbor);

      neighbor = getpixel(h + 1, w + 1);
      neighbor.r = byteCap(neighbor.r + qer * 1/16);
      neighbor.g = byteCap(neighbor.g + qeg * 1/16);
      neighbor.b = byteCap(neighbor.b + qeb * 1/16);
      setpixel(h + 1, w + 1, neighbor);

      // get the neighboring pixels and set their values up accordingly
    }
  }

}

/*
  reduce palette of the image
*/
void Image::reducePalette(std::vector<pixel> &palette) {

  for (int h = 0; h < height; ++h) {
    for (int w = 0; w < width; ++w) {
      // find the closest color and set the pixel accordingly
      pixel current_pixel = getpixel(h, w);
      int palette_index = findClosestPaletteColor(current_pixel, palette);
      setpixel(h, w, palette[palette_index]);
    }
  }

}

// root mean square of all pixel values in the given range
pixel RMS(std::vector<pixel> &pixels, size_t start, size_t end) {

	// get the total number of pixels
	double n = (double)(end - start + 1);

	double r, g, b;  // these values will store the square of the sums
	r = g = b = 0;

	// average all the channels up
	for (size_t i = start; i <= end; ++i) {

		double red = (double)pixels[i].r;
		double green = (double)pixels[i].g;
		double blue = (double)pixels[i].b;

		r += red * red;
		g += green * green;
		b += blue * blue;
	}

	// divide the sum by the number of pixels and take the square root
	r = sqrt(r / n);
	g = sqrt(g / n);
	b = sqrt(b / n);

	return pixel((uchar)r, (uchar)g, (uchar)b, 255);
}

// comparator functions for red, green and blue color channels
bool compareRed(pixel color1, pixel color2) {
	return color1.r < color2.r;
}

bool compareGreen(pixel color1, pixel color2) {
	return color1.g < color2.g;
}

bool compareBlue(pixel color1, pixel color2) {
	return color1.b < color2.b;
}

// return channel with the biggest range
Channel getBiggestRangeChannel(std::vector<pixel> &pixels, size_t start, size_t end) {

	uchar minRed, maxRed;
	uchar minGreen, maxGreen;
	uchar minBlue, maxBlue;

	minRed = minGreen = minBlue = 255;
	maxRed = maxGreen = maxBlue = 0;

	// iterate over pixels[start...end]
	for (size_t i = start; i <= end; ++i) {

		// get the current pixel value
		uchar red = pixels[i].r;
		uchar blue = pixels[i].b;
		uchar green = pixels[i].g;

		// update the mins and maxes appropriately
		if (red < minRed) minRed = red;
		if (green < minGreen) minGreen = green;
		if (blue < minBlue) minBlue = blue;

		if (red > maxRed) maxRed = red;
		if (green > maxGreen) maxGreen = green;
		if (blue > maxBlue) maxBlue = blue;
	}

	// get the maximum ranges for all colors now
	uchar redRange = maxRed - minRed;
	uchar greenRange = maxGreen - minGreen;
	uchar blueRange = maxBlue - minBlue;

	// return the color with the biggest range
	if (redRange > greenRange && redRange > blueRange) return RED;
	if (greenRange > blueRange) return GREEN;
	return BLUE;
}

// utility function to support the median cut procedure
void medianCutUtil(std::vector<pixel> &pixels, std::vector<pixel> &palette,
				   size_t start, size_t end, size_t &paletteIndex, size_t length) {

	// base case:
	if (length == 0) {
		// average all the colors out to get a single color
		pixel& color = palette[paletteIndex++];
		color = RMS(pixels, start, end);
	}
	else {
		// sort based on the color channel with the biggest range
		Channel channel = getBiggestRangeChannel(pixels, start, end);

		bool (*compare)(pixel, pixel);
		// set the appropriate comparator based on the color channel
		if (channel == RED) compare = compareRed;
		else if (channel == GREEN) compare = compareGreen;
		else compare = compareBlue;

		// sort the given range
		std::sort(pixels.begin() + start, pixels.begin() + end + 1, compare);

		// divide into two halves and let the recursion magic begin
		size_t mid = (start + end) / 2;

		// recurse on the left half and then on the right half
		medianCutUtil(pixels, palette, start, mid, paletteIndex, length - 1);
		medianCutUtil(pixels, palette, mid + 1, end, paletteIndex, length - 1);
	}
}

// apply the median cut algorithm, a thin wrapper over the main median cut procedure
void medianCut(std::vector<pixel> &pixels, std::vector<pixel> &palette) {

	size_t length = palette.size();

	size_t paletteIndex = 0;
	medianCutUtil(pixels, palette, 0, pixels.size() - 1, paletteIndex, (size_t)log2((double)length));
}

// reduce the number of colors in the image by applying the median cut algorithm
void Image::getReducedPalette(std::vector<pixel> &palette) {

  std::unordered_set<pixel, HashColor> unique; // store all the unique pixel values

  // iterate over every pixel and add it to the set if it doesn't already exist
  for (int h = 0; h < height; ++h) {
    for (int w = 0; w < width; ++w) {
      // get the current pixel
      pixel current = getpixel(h, w);

      // std::cout << "(" << (int)current.r << ", " << (int)current.g << ", " << (int)current.b << ")\n";

  		if (unique.find(current) == unique.end()) {
  			// the element does not exist, add it
  			unique.insert(current);
  		}

    }
  }

  std::vector<pixel> unique_pixels;  // vector containing all the unique pixels

  // print out the contents of the set
  for (const auto& itr : unique) {

    // std::cout << "r = " << (int)itr.r << " g = " << (int)itr.g << " b = " <<
                 // (int)itr.b << " a = " << (int)itr.a << "\n";

    unique_pixels.push_back(pixel(itr.r, itr.g, itr.b, itr.a));
  }

  std::cout << "# of colors in the original image: " << unique_pixels.size() << "\n";

  // let the median cut algorithm begin
  medianCut(unique_pixels, palette);
}
