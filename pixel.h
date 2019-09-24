/*
  a structure that helps model a pixel in a more intuitive way
*/

#ifndef PIXEL_H
#define PIXEL_H

#include <cstdlib>
#include <iostream>

enum Channel {
	RED, GREEN, BLUE
};

// model an rgba pixel
struct pixel {
    unsigned char r, g, b, a;

		pixel(): r(0), g(0), b(0), a(255) {}

    pixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : r(r), g(g), b(b), a(a) {}

    pixel operator -(pixel color) {
      return pixel(abs(r - color.r), abs(g - color.g), abs(b - color.b), 255); // leave alpha as it is for now
    }

		pixel operator +(pixel color) {
			return pixel(r + color.r, g + color.g, b + color.b, 255);
		}

		// scalar multiplication
		pixel operator * (float scalar) {
			return pixel(r * scalar, g * scalar, b * scalar, 255);
		}

    // comparator
  	bool operator==(const pixel& p) const
  	{
  		return this->r == p.r && this->g == p.g && this->b == p.b && this->a == p.a;
  	}
};

// class for hash function for the color
class HashColor {
public:
  	// id is returned as hash function
  	size_t operator()(const pixel& p) const
  	{
  		// the most sophisticated hashing scheme ever
  		return (size_t)p.r * 1000 + (size_t)p.g * 100 + (size_t)p.b * 10 + (size_t)p.a;
  	}
};

#endif
