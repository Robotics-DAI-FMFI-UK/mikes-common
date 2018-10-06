#include <string.h>
#include <stdio.h>

#include "../live/gui.h"
#include "../live/base_module.h"    /* WHEELS_DISTANCE */
#include "../passive/pose.h"
#include "mikes_logs.h"
#include "core/config_mikes.h"

#define BORDER_LEFT 20
#define BORDER_RIGHT 20
#define BORDER_TOP 20
#define BORDER_BOTTOM 20

#define POSE_ARROW_LENGTH (WHEELS_DISTANCE / 10.0)    /* in cm */

static int win;

static char *filename;
static int width;
static int height;
static int pose_visible;
static pose_type last_pose;
static double svg_map_scale;

static cairo_surface_t *svg_drawn;

// project coordinates in mm to map window pixel coordinates
#define XMM2WIN(x) ((int)(BORDER_LEFT             + (x) * svg_map_scale))
#define YMM2WIN(y) ((int)(height - 1 - BORDER_TOP - (y) * svg_map_scale))

void x_line_map_paint(cairo_t *w)
{
   cairo_push_group(w);

   cairo_set_source_rgb(w, 1, 1, 1);
   cairo_paint(w);

   if (svg_drawn == 0)
   {
     svg_drawn = cairo_surface_create_similar (cairo_get_target(w), CAIRO_CONTENT_COLOR, width - BORDER_LEFT - BORDER_RIGHT, height - BORDER_TOP - BORDER_BOTTOM); 
     cairo_t *svg_context = cairo_create(svg_drawn);
     cairo_set_source_rgb(svg_context, 1, 1, 1);
     cairo_paint(svg_context);
     svg_map_scale = draw_svg_to_cairo(svg_context, 0, 0, filename, width - BORDER_LEFT - BORDER_RIGHT, height - BORDER_TOP - BORDER_BOTTOM); 
   }
   
   cairo_set_source_surface (w, svg_drawn, BORDER_LEFT, BORDER_TOP);
   cairo_paint(w);

   if (pose_visible)
   {
     cairo_set_source_rgb(w, 0.1, 0.1, 0.5);
     cairo_set_line_width(w, 1);
     int px1_win = XMM2WIN(last_pose.x - POSE_ARROW_LENGTH / 2.0 * sin(last_pose.heading));
     int py1_win = YMM2WIN(last_pose.y - POSE_ARROW_LENGTH / 2.0 * cos(last_pose.heading));
     int px2_win = XMM2WIN(last_pose.x + POSE_ARROW_LENGTH / 2.0 * sin(last_pose.heading));
     int py2_win = YMM2WIN(last_pose.y + POSE_ARROW_LENGTH / 2.0 * cos(last_pose.heading));
     int px3_win = XMM2WIN(last_pose.x + POSE_ARROW_LENGTH / 2.0 * sin(last_pose.heading + M_PI / 2.0));
     int py3_win = YMM2WIN(last_pose.y + POSE_ARROW_LENGTH / 2.0 * cos(last_pose.heading + M_PI / 2.0));
     int px4_win = XMM2WIN(last_pose.x + POSE_ARROW_LENGTH / 2.0 * sin(last_pose.heading - M_PI / 2.0));
     int py4_win = YMM2WIN(last_pose.y + POSE_ARROW_LENGTH / 2.0 * cos(last_pose.heading - M_PI / 2.0));

     cairo_move_to(w, px2_win, py2_win);
     cairo_line_to(w, px3_win, py3_win);
     cairo_line_to(w, px4_win, py4_win);
     cairo_line_to(w, px2_win, py2_win);     
     cairo_fill(w);
     cairo_move_to(w, px1_win, py1_win);
     cairo_line_to(w, px2_win, py2_win);
     cairo_stroke(w);

     /*
     // draw map bounding rectangle (just to test correct X,Y transformations)
     int mx1 = XMM2WIN(11 * 10.0);
     int my1 = YMM2WIN(12.000005 * 10.0);
     int mx2 = XMM2WIN(591 * 10.0);
     int my2 = YMM2WIN(339 * 10.0);

     cairo_move_to(w, mx1, my1);
     cairo_line_to(w, mx2, my1);
     cairo_line_to(w, mx2, my2);
     cairo_line_to(w, mx1, my2);
     cairo_line_to(w, mx1, my1);
     */

   }
   cairo_pop_group_to_source(w);
   cairo_paint(w);
}

void x_line_map_toggle_pose_visible(int visible)
{
   if (pose_visible != visible)
   {
       pose_visible = visible;
       request_async_repaint(win);
   }
}

void x_line_map_set_pose(pose_type p)
{
   last_pose = p;
   if (pose_visible) request_async_repaint(win);
}

void line_map_mouse_listener(int x, int y, int button)
{
   double map_x = (x - BORDER_LEFT) / svg_map_scale;  // * 10.0  // assuming dimensions in svg are centimeters
   double map_y = (y - BORDER_TOP) / svg_map_scale;   // * 10.0  // calculated coordinates are in centimeters
   printf("line map click: win_x=%d, win_y=%d, map_x=%.3lf, map_y=%.3lf\n", x, y, map_x, map_y);
}

void init_x_line_map(char *svg_filename, int win_width, int win_height)
{
   if (!mikes_config.with_gui) return;
   filename = svg_filename;
   pose_visible = 0;
   svg_drawn = 0;
   width = win_width;
   height = win_height;
   win = gui_open_window(x_line_map_paint, win_width, win_height, 0);
   gui_set_window_title(win, "MAP");
   gui_add_mouse_listener(win, line_map_mouse_listener);
}

void shutdown_x_line_map()
{
   if (!mikes_config.with_gui) return;
   gui_close_window(win);
}

