#ifndef __PNGWRITER_H__
#define __PNGWRITER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" void write_greyscale_png_image(const short* pixels, char *filename, int w, int h, double color_divider);
extern "C" void write_greyscale_png_image_with_min(const short* pixels, char *filename, int w, int h, double color_divider, uint8_t color_offset);
extern "C" void write_rgb_png_image(const short* pixels, char *filename, int w, int h);
#else
void write_greyscale_png_image(const short* pixels, char *filename, int w, int h, double color_divider);
void write_greyscale_png_image_with_min(const short* pixels, char *filename, int w, int h, double color_divider, uint8_t color_offset);
void write_rgb_png_image(const short* pixels, char *filename, int w, int h);
#endif

#endif 

