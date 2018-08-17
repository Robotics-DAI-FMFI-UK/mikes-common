#include <string.h>
#include <stdio.h>

#include "x_ust10lx.h"
#include "../live/gui.h"

#define ENLARGE_CENTER 50

#define RAY_USUAL_TYPE 1
#define RAY_AZIMUTH_TYPE 2
#define RAY_ZERO_TYPE 3

static int win;
static uint16_t            dist_local_copy[UST10LX_DATA_COUNT];
static int range;
static double scale_factor;

void draw_ray(cairo_t *w, int i, uint16_t d, int ray_type)
{
   int x = 0, y=0, center_x=0, center_y=0;
   double alpha = M_PI * ust10lx_ray2azimuth(i) / 180.0;

   if (ray_type == RAY_USUAL_TYPE)
   {
      x = (int)((ENLARGE_CENTER + d) / scale_factor * X_UST10LX_WIDTH * 0.45 * sin(alpha) + X_UST10LX_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + d) / scale_factor * X_UST10LX_HEIGHT * 0.45 * cos(alpha) + X_UST10LX_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_UST10LX_WIDTH * 0.45 * sin(alpha) + X_UST10LX_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_UST10LX_HEIGHT * 0.45 * cos(alpha) + X_UST10LX_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.1, 0.1, 0.3 + 0.7);
   } else if (ray_type == RAY_AZIMUTH_TYPE)
   {
      x = (int)((ENLARGE_CENTER + range) / scale_factor * X_UST10LX_WIDTH * 0.45 * sin(M_PI * i / 64.0 / 180.0) + X_UST10LX_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + range) / scale_factor * X_UST10LX_HEIGHT * 0.45 * cos(M_PI * i / 64.0 / 180.0) + X_UST10LX_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_UST10LX_WIDTH * 0.45 * sin(M_PI * i / 64.0 / 180.0) + X_UST10LX_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_UST10LX_WIDTH * 0.45 * cos(M_PI * i / 64.0 / 180.0) + X_UST10LX_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.8, 0.8, 0.3);
   } else if (ray_type == RAY_ZERO_TYPE)
   {
      x = (int)((ENLARGE_CENTER + range) / scale_factor * X_UST10LX_WIDTH * 0.45 * sin(alpha) + X_UST10LX_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + range) / scale_factor * X_UST10LX_HEIGHT * 0.45 * cos(alpha) + X_UST10LX_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_UST10LX_WIDTH * 0.45 * sin(alpha) + X_UST10LX_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_UST10LX_HEIGHT * 0.45 * cos(alpha) + X_UST10LX_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.8, 0.8, 0.8);
   }
   
   cairo_move_to(w, x, X_UST10LX_HEIGHT - y);
   cairo_line_to(w, center_x, X_UST10LX_HEIGHT - center_y);
   cairo_stroke(w);
   cairo_set_source_rgb(w, 1, 0.3, 0.3);
   cairo_arc(w, x, X_UST10LX_HEIGHT - y, 2, 0, 2 * M_PI);
   cairo_stroke(w);
}

void x_ust10lx_paint(cairo_t *w)
{
   cairo_push_group(w);

   cairo_set_source_rgb(w, 1, 1, 1);
   cairo_paint(w);
   cairo_set_line_width(w, 1);
    
   for (int i = 0; i < UST10LX_DATA_COUNT; i++)
   {
      uint16_t d = dist_local_copy[i];

      if (d > range) d = range;
      draw_ray(w, i, d, (d == 65533) ? RAY_ZERO_TYPE : RAY_USUAL_TYPE);
   }
  
   cairo_pop_group_to_source(w);
   cairo_paint(w);
}

void x_ust10lx_update(uint16_t *dist)
{
   memcpy(dist_local_copy, dist, sizeof(uint16_t) * UST10LX_DATA_COUNT);
}

void init_x_ust10lx(int max_range_in_mm, int window_update_period_in_ms)
{
   range = max_range_in_mm;
   scale_factor = range;

   win = gui_open_window(x_ust10lx_paint, X_UST10LX_WIDTH, X_UST10LX_HEIGHT, window_update_period_in_ms);
   gui_set_window_title(win, "UST10LX");
   register_ust10lx_callback(x_ust10lx_update); 
}

void shutdown_x_ust10lx()
{
   gui_close_window(win);
}

