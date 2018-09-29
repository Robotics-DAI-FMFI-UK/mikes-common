#include <string.h>
#include <stdio.h>

#include "x_tim571.h"
#include "../live/gui.h"
#include "mikes_logs.h"
#include "core/config_mikes.h"

#define ENLARGE_CENTER 50

#define RAY_USUAL_TYPE 1
#define RAY_AZIMUTH_TYPE 2
#define RAY_ZERO_TYPE 3

#define BASIC_LINE 1
#define FILTERED_LINE 2

int show_raw_lines;

static int win;
static uint16_t             dist_local_copy[TIM571_DATA_COUNT];
static uint8_t              rssi_local_copy[TIM571_DATA_COUNT];
static tim571_status_data   status_local_copy;
static lines_data           lines_local;
static lines_data           lines_filtered_local;
static int range;
static double scale_factor;

void tim571_draw_ray(cairo_t *w, int i, uint16_t d, uint8_t q, int ray_type)
{
   int x = 0, y=0, center_x=0, center_y=0;
   double alpha = M_PI * tim571_ray2azimuth(i) / 180.0;
   //printf("%5.3lf (%5.3lf): %d (%d)\n", tim571_ray2azimuth(i), alpha, d, q);

   if (ray_type == RAY_USUAL_TYPE)
   {
      x = (int)((ENLARGE_CENTER + d) / scale_factor * X_TIM571_WIDTH * 0.45 * sin(alpha) + X_TIM571_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + d) / scale_factor * X_TIM571_HEIGHT * 0.45 * cos(alpha) + X_TIM571_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_TIM571_WIDTH * 0.45 * sin(alpha) + X_TIM571_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_TIM571_HEIGHT * 0.45 * cos(alpha) + X_TIM571_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.7 * (255 - q) / 255, 0.7 * (255 - q) / 255, 0.3 + 0.7 * q / 255.0);
   } else if (ray_type == RAY_AZIMUTH_TYPE)
   {
      x = (int)((ENLARGE_CENTER + range) / scale_factor * X_TIM571_WIDTH * 0.45 * sin(M_PI * i / 64.0 / 180.0) + X_TIM571_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + range) / scale_factor * X_TIM571_HEIGHT * 0.45 * cos(M_PI * i / 64.0 / 180.0) + X_TIM571_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_TIM571_WIDTH * 0.45 * sin(M_PI * i / 64.0 / 180.0) + X_TIM571_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_TIM571_WIDTH * 0.45 * cos(M_PI * i / 64.0 / 180.0) + X_TIM571_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.8, 0.8, 0.3);
   } else if (ray_type == RAY_ZERO_TYPE)
   {
      x = (int)((ENLARGE_CENTER + range) / scale_factor * X_TIM571_WIDTH * 0.45 * sin(alpha) + X_TIM571_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + range) / scale_factor * X_TIM571_HEIGHT * 0.45 * cos(alpha) + X_TIM571_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_TIM571_WIDTH * 0.45 * sin(alpha) + X_TIM571_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_TIM571_HEIGHT * 0.45 * cos(alpha) + X_TIM571_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.8, 0.8, 0.8);
   }

   cairo_move_to(w, x, X_TIM571_HEIGHT - y);
   cairo_line_to(w, center_x, X_TIM571_HEIGHT - center_y);
   cairo_stroke(w);
   if (ray_type == RAY_USUAL_TYPE)
      cairo_set_source_rgb(w, q / 255.0, 0.7 * (255 - q) / 255, 0.3);
   else if (ray_type == RAY_AZIMUTH_TYPE)
      cairo_set_source_rgb(w, 0.8, 0.8, 0.3);
   else if (ray_type == RAY_ZERO_TYPE)
      cairo_set_source_rgb(w, 0.8, 0.8, 0.8);
   else cairo_set_source_rgb(w, 1, 0.3, 0.3);

   cairo_arc(w, x, X_TIM571_HEIGHT - y, 2, 0, 2 * M_PI);
   cairo_stroke(w);
}

void tim571_draw_line(cairo_t *w, line_data *ln, int line_type)
{
   uint16_t d = ln->distance;
   double alpha = (90 - ln->angle) / 180.0 * M_PI;

   int x = (int)((ENLARGE_CENTER + d) / scale_factor * X_TIM571_WIDTH * 0.45 * sin(alpha) + X_TIM571_WIDTH / 2);
   int y = (int)((ENLARGE_CENTER + d) / scale_factor * X_TIM571_HEIGHT * 0.45 * cos(alpha) + X_TIM571_HEIGHT / 2);

   if (line_type == BASIC_LINE)
     cairo_set_source_rgb(w, 0.8, 0.8, 0.3);
   else
     cairo_set_source_rgb(w, 0.3, 0.5, 0.5);

   double slen = sqrt((x - X_TIM571_WIDTH / 2) * (x - X_TIM571_WIDTH / 2) + (y - X_TIM571_HEIGHT / 2) * (y - X_TIM571_HEIGHT / 2));
   if (slen < 0.000001) return;

   double sx = (x - X_TIM571_WIDTH / 2) / slen;
   double sy = (y - X_TIM571_HEIGHT / 2) / slen;

   int x1 = (int)(x - (-sy) * (X_TIM571_WIDTH + X_TIM571_HEIGHT) + 0.5);
   int y1 = (int)(y - sx * (X_TIM571_WIDTH + X_TIM571_HEIGHT) + 0.5);
   int x2 = (int)(x + (-sy) * (X_TIM571_WIDTH + X_TIM571_HEIGHT) + 0.5);
   int y2 = (int)(y + sx * (X_TIM571_WIDTH + X_TIM571_HEIGHT) + 0.5);

   cairo_move_to(w, x1, X_TIM571_HEIGHT - y1);
   cairo_line_to(w, x2, X_TIM571_HEIGHT - y2);

   cairo_stroke(w);
}

void x_tim571_paint(cairo_t *w)
{
   cairo_push_group(w);

   cairo_set_source_rgb(w, 1, 1, 1);
   cairo_paint(w);
   cairo_set_line_width(w, 1);

   for (int i = 0; i < TIM571_DATA_COUNT; i++)
   {
      uint16_t d = dist_local_copy[i];
      uint8_t  r = rssi_local_copy[i];

      if (d > range) d = range;
      tim571_draw_ray(w, i, d, r, (d == 0) ? RAY_ZERO_TYPE : RAY_USUAL_TYPE);
   }

   if (show_raw_lines)
     for (int i = 0; i < lines_local.line_count; i++)
        tim571_draw_line(w, lines_local.lines + i, BASIC_LINE);

   for (int i = 0; i < lines_filtered_local.line_count; i++)
      tim571_draw_line(w, lines_filtered_local.lines + i, FILTERED_LINE);

   cairo_pop_group_to_source(w);
   cairo_paint(w);
}

void x_tim571_laser_update(uint16_t *dist, uint8_t *rssi, tim571_status_data *status_data)
{
   memcpy(dist_local_copy, dist, sizeof(uint16_t) * TIM571_DATA_COUNT);
   memcpy(rssi_local_copy, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
}

void x_tim571_lines_update(lines_data *lines)
{
  lines_local = *lines;
}

void filtered_lines_update(lines_data *lines)
{
  lines_filtered_local = *lines;
}

void init_x_tim571(int max_range_in_mm, int window_update_period_in_ms)
{
   show_raw_lines = 1;

   if (!mikes_config.with_gui) return;
   if (!mikes_config.use_tim571)
   {
        mikes_log(ML_INFO, "tim571 gui supressed by config.");
        return;
   }

   range = max_range_in_mm;
   scale_factor = range;

   win = gui_open_window(x_tim571_paint, X_TIM571_WIDTH, X_TIM571_HEIGHT, window_update_period_in_ms);
   gui_set_window_title(win, "TIM571");
   register_tim571_callback(x_tim571_laser_update);
   register_tim_hough_transform_callback(x_tim571_lines_update);
   register_line_filter_callback(filtered_lines_update);
   get_tim571_status_data(&status_local_copy);
}

void shutdown_x_tim571()
{
   if (!mikes_config.with_gui) return;
   if (!mikes_config.use_tim571)
   gui_close_window(win);
}
