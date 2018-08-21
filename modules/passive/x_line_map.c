#include <string.h>
#include <stdio.h>

#include "../live/gui.h"
#include "../passive/pose.h"

#define BORDER_LEFT 20
#define BORDER_RIGHT 20
#define BORDER_TOP 20
#define BORDER_BOTTOM 20

#define POSE_ARROW_LENGTH 450

static int win;

static char *filename;
static int width;
static int height;
static int pose_visible;
static pose_type last_pose;
static double svg_map_scale;

static cairo_surface_t *svg_drawn;

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
     int px1_win = (int)(BORDER_LEFT + (last_pose.x - POSE_ARROW_LENGTH / 2.0 * sin(last_pose.heading)) * svg_map_scale / 10.0);
     int py1_win = (int)(BORDER_TOP + (last_pose.y + POSE_ARROW_LENGTH / 2.0 * cos(last_pose.heading)) * svg_map_scale / 10.0);
     int px2_win = (int)(BORDER_LEFT + (last_pose.x + POSE_ARROW_LENGTH / 2.0 * sin(last_pose.heading)) * svg_map_scale / 10.0);
     int py2_win = (int)(BORDER_TOP + (last_pose.y - POSE_ARROW_LENGTH / 2.0 * cos(last_pose.heading)) * svg_map_scale / 10.0);
     int px3_win = (int)(BORDER_LEFT + (last_pose.x + POSE_ARROW_LENGTH / 2.0 * sin(last_pose.heading + M_PI / 2.0)) * svg_map_scale / 10.0);
     int py3_win = (int)(BORDER_TOP + (last_pose.y - POSE_ARROW_LENGTH / 2.0 * cos(last_pose.heading + M_PI / 2.0)) * svg_map_scale / 10.0);
     int px4_win = (int)(BORDER_LEFT + (last_pose.x + POSE_ARROW_LENGTH / 2.0 * sin(last_pose.heading - M_PI / 2.0)) * svg_map_scale / 10.0);
     int py4_win = (int)(BORDER_TOP + (last_pose.y - POSE_ARROW_LENGTH / 2.0 * cos(last_pose.heading - M_PI / 2.0)) * svg_map_scale / 10.0);

     cairo_move_to(w, px1_win, py1_win);
     cairo_line_to(w, px2_win, py2_win);

     cairo_line_to(w, px3_win, py3_win);
     cairo_move_to(w, px2_win, py2_win);
     cairo_line_to(w, px4_win, py4_win);

     cairo_stroke(w);
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
   double map_x = (x - BORDER_LEFT) / svg_map_scale * 10.0;  // assuming dimensions in svg are centimeters
   double map_y = (y - BORDER_TOP) / svg_map_scale * 10.0;
   printf("line map click: win_x=%d, win_y=%d, map_x=%.3lf, map_y=%.3lf\n", x, y, map_x, map_y);
}

void init_x_line_map(char *svg_filename, int win_width, int win_height)
{
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
   gui_close_window(win);
}

