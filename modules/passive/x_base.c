#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../live/gui.h"
#include "x_base.h"
#include "../live/base_module.h"

#define COMPASS_RADIUS ((X_BASE_HEIGHT<X_BASE_WIDTH)?(X_BASE_HEIGHT*0.39):(X_BASE_WIDTH*0.39))

#define COMPASS_POINTER 1
#define AZIMUTH_POINTER 2

static int win;
static base_data_type base_local_copy;
static double azimuth;

void draw_compass_cross(cairo_t *w)
{
   cairo_set_source_rgba(w, 0.9, 0.9, 0.9, 0.9);
   cairo_set_line_width(w, 1);
   cairo_move_to(w, X_BASE_WIDTH / 2.0, X_BASE_HEIGHT - (X_BASE_HEIGHT / 2.0 + 0.97 * COMPASS_RADIUS));
   cairo_line_to(w, X_BASE_WIDTH / 2.0, X_BASE_HEIGHT - (X_BASE_HEIGHT / 2.0 - 0.97 * COMPASS_RADIUS));
   cairo_move_to(w, X_BASE_WIDTH / 2.0 - 0.97 * COMPASS_RADIUS, X_BASE_HEIGHT / 2.0);
   cairo_line_to(w, X_BASE_WIDTH / 2.0 + 0.97 * COMPASS_RADIUS, X_BASE_HEIGHT / 2.0);
   cairo_stroke(w);
}

void draw_compass_cylinder(cairo_t *w)
{
   char deg_str[10];
   cairo_text_extents_t text_extents;

   cairo_set_source_rgb(w, 0, 0, 1);
   cairo_set_line_width(w, 1);
   cairo_set_font_size(w, 10);

   cairo_arc(w, X_BASE_WIDTH / 2, X_BASE_HEIGHT / 2, COMPASS_RADIUS, 0, 2 * M_PI);
   for (int i = 0; i < 36; i++)
   {
     cairo_move_to(w, X_BASE_WIDTH / 2.0 + 0.97 * COMPASS_RADIUS * sin((i * 10.0) / 180.0 * M_PI),
                      X_BASE_HEIGHT - (X_BASE_HEIGHT / 2.0 + 0.97 * COMPASS_RADIUS * cos((i * 10.0) / 180.0 * M_PI)));
     cairo_line_to(w, X_BASE_WIDTH / 2.0 + 1.01 * COMPASS_RADIUS * sin((i * 10.0) / 180.0 * M_PI),
                      X_BASE_HEIGHT - (X_BASE_HEIGHT / 2.0 + 1.01 * COMPASS_RADIUS * cos((i * 10.0) / 180.0 * M_PI)));
     sprintf(deg_str, "%d", i * 10);
     cairo_text_extents(w, deg_str, &text_extents);

     cairo_move_to(w, X_BASE_WIDTH / 2.0 - text_extents.width / 2.0 + 1.14 * COMPASS_RADIUS * sin((i * 10.0) / 180.0 * M_PI),
                      X_BASE_HEIGHT - (X_BASE_HEIGHT / 2.0  - text_extents.height / 2.0 + 1.14 * COMPASS_RADIUS * cos((i * 10.0) / 180.0 * M_PI)));
     cairo_show_text(w, deg_str);
   }
   cairo_stroke(w);

   draw_compass_cross(w);
}

void draw_compass_pointer(cairo_t *w, int compass_pointer_type)
{
   double a = base_local_copy.heading;
   if (compass_pointer_type == COMPASS_POINTER)
     cairo_set_source_rgba(w, 0, 0.7, 0, 0.9);
   else if (compass_pointer_type == AZIMUTH_POINTER)
   {
     cairo_set_source_rgba(w, 0.7, 0.5, 0.5, 0.9);
     a = azimuth;
   }

   cairo_set_line_width(w, 2);

   cairo_move_to(w, X_BASE_WIDTH / 2, X_BASE_HEIGHT / 2);
   cairo_line_to(w, X_BASE_WIDTH / 2.0 + 0.95 * COMPASS_RADIUS * sin(a / 180.0 * M_PI),
                    X_BASE_HEIGHT - (X_BASE_HEIGHT / 2.0 + 0.95 * COMPASS_RADIUS * cos(a / 180.0 * M_PI)));

   cairo_stroke(w);
}

void draw_rotations(cairo_t *w)
{
   char rot[15];

   cairo_set_source_rgb(w, 0.8, 0.2, 0.2);
   cairo_set_font_size(w, 10);

   cairo_move_to(w, X_BASE_WIDTH / 20, X_BASE_HEIGHT - 12);
   sprintf(rot, "A: %ld", base_local_copy.counterA);
   cairo_show_text(w, rot);

   cairo_move_to(w, X_BASE_WIDTH * 8 / 10, X_BASE_HEIGHT - 12);
   sprintf(rot, "B: %ld", base_local_copy.counterB);
   cairo_show_text(w, rot);
   cairo_stroke(w);
}

void show_timestamp(cairo_t *w)
{
   char stamp[15];

   cairo_set_source_rgb(w, 0.6, 0.6, 0.4);
   cairo_set_font_size(w, 10);

   cairo_move_to(w, X_BASE_WIDTH * 8 / 10, 14);
   sprintf(stamp, "%.2lf", base_local_copy.timestamp / 1000000.0);
   cairo_show_text(w, stamp);
   cairo_stroke(w);
}

void x_base_paint(cairo_t *w)
{
   cairo_push_group(w);

   cairo_set_source_rgb(w, 1, 1, 1);
   cairo_paint(w);

   draw_compass_cylinder(w);
   draw_compass_pointer(w, COMPASS_POINTER);
   if (azimuth != AZIMUTH_NOT_SET)
     draw_compass_pointer(w, AZIMUTH_POINTER);

   draw_rotations(w);
   show_timestamp(w);
  
   cairo_pop_group_to_source(w);
   cairo_paint(w);
}

void x_base_update(base_data_type *data)
{
   memcpy(&base_local_copy, data, sizeof(base_data_type));
}

void x_base_set_azimuth(double new_azimuth)
{
  azimuth = new_azimuth;
}

void init_x_base(int window_update_period_in_ms)
{
   azimuth = AZIMUTH_NOT_SET;
   get_base_data(&base_local_copy);
   win = gui_open_window(x_base_paint, X_BASE_WIDTH, X_BASE_HEIGHT, window_update_period_in_ms);
   gui_set_window_title(win, "BASE");
   register_base_callback(x_base_update); 
}

void shutdown_x_base()
{
   gui_close_window(win);
}

