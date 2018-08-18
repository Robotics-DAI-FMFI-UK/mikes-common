#include "../bites/pngwriter.h"

void test_write_greyscale()
{
  short grey_pixels[320 * 200];

  for (int r = 0; r < 200; r++)
    for (int c = 0; c < 320; c++)
      grey_pixels[r * 320 + c] = (r + c);

  write_greyscale_png_image(grey_pixels, "grey_gradient.png", 320, 200, (319 + 199) / 255.0);

}

void test_write_rgb()
{
  short rgb_pixels[320 * 200 * 3];

  for (int r = 0; r < 200; r++)
  {
    for (int c = 0; c < 10; c++)
    {
      rgb_pixels[r * 320 * 3 + c * 3] = 200 - r;
      rgb_pixels[r * 320 * 3 + c * 3 + 1] = 200 - r;
      rgb_pixels[r * 320 * 3 + c * 3 + 2] = 200 - r;

      rgb_pixels[r * 320 * 3 + c * 3 + 310 * 3] = r;
      rgb_pixels[r * 320 * 3 + c * 3 + 310 * 3 + 1] = r;
      rgb_pixels[r * 320 * 3 + c * 3 + 310 * 3 + 2] = r;
    }

    for (int c = 10; c < 110;  c++)
    {
      rgb_pixels[r * 320 * 3 + c * 3] = r;
      rgb_pixels[r * 320 * 3 + c * 3 + 1] = 0;
      rgb_pixels[r * 320 * 3 + c * 3 + 2] = 0;
    }

    for (int c = 110; c < 210;  c++)
    {
      rgb_pixels[r * 320 * 3 + c * 3] = 0;
      rgb_pixels[r * 320 * 3 + c * 3 + 1] = 200 - r;
      rgb_pixels[r * 320 * 3 + c * 3 + 2] = 0;
    }

    for (int c = 210; c < 310;  c++)
    {
      rgb_pixels[r * 320 * 3 + c * 3] = 0;
      rgb_pixels[r * 320 * 3 + c * 3 + 1] = 0;
      rgb_pixels[r * 320 * 3 + c * 3 + 2] = 200 - r;
    }
  }

  write_rgb_png_image(rgb_pixels, "rgb_gradient.png", 320, 200);
}

int main()
{
  test_write_greyscale();
  test_write_rgb();
  return 0;
}

