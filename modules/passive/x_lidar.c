#include <string.h>
#include <stdio.h>
#include <math.h>

#include "x_lidar.h"
#include "../live/gui.h"

#define ENLARGE_CENTER 50

#define RAY_USUAL_TYPE 1
#define RAY_AZIMUTH_TYPE 2
#define RAY_ZERO_TYPE 3

static int win;
static lidar_data_type data;
static int range;
static double scale_factor;

void draw_ray(cairo_t *w, int i, uint16_t d, double alpha, int ray_type)
{
   int x = 0, y=0, center_x=0, center_y=0;
   alpha = M_PI * alpha / 180.0;

   if (ray_type == RAY_USUAL_TYPE)
   {
      x = (int)((ENLARGE_CENTER + d) / scale_factor * X_LIDAR_WIDTH * 0.45 * sin(alpha) + X_LIDAR_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + d) / scale_factor * X_LIDAR_HEIGHT * 0.45 * cos(alpha) + X_LIDAR_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_LIDAR_WIDTH * 0.45 * sin(alpha) + X_LIDAR_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_LIDAR_HEIGHT * 0.45 * cos(alpha) + X_LIDAR_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.1, 0.1, 0.3 + 0.7);
   } else if (ray_type == RAY_AZIMUTH_TYPE)
   {
      x = (int)((ENLARGE_CENTER + range) / scale_factor * X_LIDAR_WIDTH * 0.45 * sin(M_PI * i / 64.0 / 180.0) + X_LIDAR_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + range) / scale_factor * X_LIDAR_HEIGHT * 0.45 * cos(M_PI * i / 64.0 / 180.0) + X_LIDAR_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_LIDAR_WIDTH * 0.45 * sin(M_PI * i / 64.0 / 180.0) + X_LIDAR_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_LIDAR_WIDTH * 0.45 * cos(M_PI * i / 64.0 / 180.0) + X_LIDAR_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.8, 0.8, 0.3);
   } else if (ray_type == RAY_ZERO_TYPE)
   {
      x = (int)((ENLARGE_CENTER + range) / scale_factor * X_LIDAR_WIDTH * 0.45 * sin(alpha) + X_LIDAR_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + range) / scale_factor * X_LIDAR_HEIGHT * 0.45 * cos(alpha) + X_LIDAR_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_LIDAR_WIDTH * 0.45 * sin(alpha) + X_LIDAR_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_LIDAR_HEIGHT * 0.45 * cos(alpha) + X_LIDAR_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.8, 0.8, 0.8);
   }
   
   cairo_move_to(w, x, X_LIDAR_HEIGHT - y);
   cairo_line_to(w, center_x, X_LIDAR_HEIGHT - center_y);
   cairo_stroke(w);
   cairo_set_source_rgb(w, 1, 0.3, 0.3);
   cairo_arc(w, x, X_LIDAR_HEIGHT - y, 2, 0, 2 * M_PI);
   cairo_stroke(w);
}

void x_lidar_paint(cairo_t *w)
{
   cairo_push_group(w);

   cairo_set_source_rgb(w, 1, 1, 1);
   cairo_paint(w);
   cairo_set_line_width(w, 1);
    
   for (int i = 0; i < data.count; i++)
   {
      uint16_t d = data.distance[i];
      uint8_t q = data.quality[i];
      double alpha = data.angle[i] / 64.0;

      if (d > range) d = range;

      draw_ray(w, i, d, alpha, (q == 0) ? RAY_ZERO_TYPE : RAY_USUAL_TYPE);
   }
  
   cairo_pop_group_to_source(w);
   cairo_paint(w);
}

void x_lidar_update(lidar_data_type *newdata)
{
   memcpy(&data, newdata, sizeof(lidar_data_type));
}

void init_x_lidar(int max_range_in_mm, int window_update_period_in_ms)
{
   range = max_range_in_mm;
   scale_factor = range;

   win = gui_open_window(x_lidar_paint, X_LIDAR_WIDTH, X_LIDAR_HEIGHT, window_update_period_in_ms);
   gui_set_window_title(win, "LIDAR");
   register_lidar_callback(x_lidar_update); 
}

void shutdown_x_lidar()
{
   gui_close_window(win);
}

