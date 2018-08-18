#include <string.h>
#include <stdio.h>

#include "../live/gui.h"

static int win;

static char *filename;
static int width;
static int height;

void x_line_map_paint(cairo_t *w)
{
   cairo_push_group(w);

   cairo_set_source_rgb(w, 1, 1, 1);
   cairo_paint(w);
   draw_svg_to_win(win, 20, 20, filename, width - 40, height - 40); 
   cairo_pop_group_to_source(w);
   cairo_paint(w);
}

void init_x_line_map(char *svg_filename, int win_width, int win_height)
{
   filename = svg_filename;
   width = win_width;
   height = win_height;
   win = gui_open_window(x_line_map_paint, win_width, win_height, 0);
   gui_set_window_title(win, "MAP");
}

void shutdown_x_line_map()
{
   gui_close_window(win);
}

