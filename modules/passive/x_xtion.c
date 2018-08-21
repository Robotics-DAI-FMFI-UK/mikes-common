#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "x_xtion.h"
#include "../live/gui.h"
#include "../../bites/pngwriter.h"
#include "../../bites/mikes.h"
#include "mikes_logs.h"
#include "core/config_mikes.h"

static int win;

static short *data;
static int data_width;
static int data_height;
static double data_zoom;

static uint8_t *img_data;
static int img_width;
static int img_height;
static int stride;
static cairo_surface_t *cairo_image;

void zoom_data_to_img()
{
   uint8_t *bmp = img_data;

   for (int r = 0; r < img_height; r++)
   {
     int ind = 0;
     for (int c = 0; c < img_width; c++)
     {
       int row = (int)(r / data_zoom + 0.5);
       int col = (int)(c / data_zoom + 0.5);
       if (row >= data_height) row = data_height - 1;
       if (col >= data_width) col = data_width - 1;

       short d = data[row * data_width + col];
       if (d == 6000) 
       {
         bmp[ind] = 30;
         bmp[ind + 1] = 0;
         bmp[ind + 2] = 0;
       }
       else
       {
         if (d > 6000) d = 6000;
         uint8_t pixel = (uint8_t)(255 * (d / 6000.0 * 0.85 + 0.05)); 

         bmp[ind] = pixel;
         bmp[ind + 1] = pixel;
         bmp[ind + 2] = pixel;
       }
       ind += 4;
     }
     bmp += stride;
   }
}

cairo_surface_t *acquire_callback(cairo_pattern_t *pattern,
                                       void *callback_data,
                                       cairo_surface_t *target,
                                       const cairo_rectangle_int_t *extents)
{
  if (cairo_image) cairo_surface_destroy(cairo_image);

  get_xtion_data(data);
  zoom_data_to_img();
  cairo_image =  cairo_image_surface_create_for_data (img_data, CAIRO_FORMAT_RGB24, img_width, img_height, stride);
  return cairo_image;
}

void release_callback(cairo_pattern_t *pattern, void *callback_data, cairo_surface_t *surface)
{
  if (cairo_image) cairo_surface_destroy(cairo_image);
  cairo_image = 0;
}

void x_xtion_paint(cairo_t *w)
{
   cairo_push_group(w);

   cairo_pattern_t *bitmap = cairo_pattern_create_raster_source(0, CAIRO_CONTENT_COLOR, 
                                                                (int)(data_width * data_zoom + 0.5),
                                                                (int)(data_height * data_zoom + 0.5));
   cairo_raster_source_pattern_set_acquire(bitmap, acquire_callback, release_callback);
   cairo_set_source(w, bitmap);
   cairo_paint(w);
    
   cairo_pop_group_to_source(w);
   cairo_paint(w);
}

void xtion_key_listener(int win, int key)
{
   if (key == GUI_ENTER_KEY)
   {
     char filename[30];
     time_t tm;
     time(&tm);
     sprintf(filename, "%ld_xtion.png", tm);
     write_greyscale_png_image_with_min(data, filename, data_width, data_height, 7060 / 255.0, 13);
     printf("wrote %s\n", filename);
   }
   else if (key == GUI_ESC_KEY) program_runs = 0;
}

void init_x_xtion(int width, int height, double zoom, int window_update_period_in_ms)
{
   if (!mikes_config.with_gui) return;
   if (!mikes_config.use_xtion)
   {
        mikes_log(ML_INFO, "asus xtion gui supressed by config.");
        return;
   }

   data_width = width;
   data_height = height;
   data_zoom = zoom;

   img_width = (int)(width * zoom + 0.5);
   img_height = (int)(height * zoom + 0.5);

   stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, img_width);
   cairo_image = 0;

   data = (short *)malloc(width * height * sizeof(short));
   img_data = (uint8_t *)malloc(stride * img_height);
   for (int i = 0; i < img_width * img_height * 4; i++) img_data[i] = 0;

   win = gui_open_window(x_xtion_paint, img_width, img_height, window_update_period_in_ms);
   gui_set_window_title(win, "ASUS XTION PRO");
   gui_add_key_listener("saving", xtion_key_listener);
}

void shutdown_x_xtion()
{
   if (!mikes_config.with_gui) return;
   if (!mikes_config.use_xtion) return;
   gui_close_window(win);
}

