#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../modules/live/gui.h"
#include "bites/mikes.h"

#define WIN1_WIDTH  600
#define WIN1_HEIGHT 400

#define WIN2_WIDTH  300
#define WIN2_HEIGHT 300

#define WIN3_WIDTH  400
#define WIN3_HEIGHT 200

#define WIN4_WIDTH  240
#define WIN4_HEIGHT 240 

int win1, win2, win3, win4;
int x1, yy1;
int x2, y2;
int x3, y3;

void mouse1(int x, int y, int button)
{
    if (strcmp(get_current_gui_context(), "drawing") != 0) return;

    cairo_t *win = get_cairo_t(win1);

    cairo_push_group(win);

    cairo_set_source_rgba(win, 0, 0, 1, 0.4);
    cairo_set_line_width(win, 1);

    cairo_move_to(win, x1, yy1);
    cairo_line_to(win, x, y);
    cairo_stroke(win);
 
    cairo_pop_group_to_source(win);
    cairo_paint(win);

    x1 = x;
    yy1 = y;
}

void mouse2(int x, int y, int button)
{
    if (strcmp(get_current_gui_context(), "drawing") != 0) return;
    cairo_t *win = get_cairo_t(win2);

    cairo_push_group(win);

    cairo_set_source_rgba(win, 0, 1, 0, 0.4);
    cairo_set_line_width(win, 3);

    cairo_move_to(win, x2, y2);
    x = rand() % WIN2_WIDTH;
    y = rand() % WIN2_HEIGHT;
    cairo_line_to(win, x, y);
    cairo_stroke(win);

    cairo_pop_group_to_source(win);
    cairo_paint(win);

    x2 = x;
    y2 = y;
}

void line3_from(int x, int y)
{
    cairo_t *win = get_cairo_t(win3);
    cairo_set_source_rgba(win, 1, 0, 0, 0.6);
    cairo_set_line_width(win, 4);
    
    cairo_move_to(win, x, y);
    cairo_line_to(win, x3, y3);
    cairo_stroke(win);
}

void paint1(cairo_t *win)
{
    cairo_push_group(win);

    cairo_set_source_rgba(win, 0.7, 0.7, 0, 0.4);
    cairo_set_line_width(win, 1);
    int x = rand() % (WIN1_WIDTH - 30);
    int y = rand() % (WIN1_HEIGHT - 20);
    int w = rand() % 30;
    int h = rand() % 20;
    cairo_rectangle(win, x, y, w, h);
    cairo_fill(win);
 
    cairo_pop_group_to_source(win);
    cairo_paint(win);
}

void paint2(cairo_t *win)
{
    cairo_push_group(win);

    cairo_select_font_face(win, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_source_rgba(win, 0.1, 0.5, 0.5, 0.5);
    cairo_set_font_size(win, 12);
    int x = rand() % (WIN2_WIDTH - 30);
    int y = rand() % (WIN2_HEIGHT - 12);
    cairo_move_to(win, x, y);
    cairo_show_text(win, "hi!");
 
    cairo_pop_group_to_source(win);
    cairo_paint(win);
}

void paint3(cairo_t *win)
{
    cairo_push_group(win);

    cairo_set_source_rgba(win, 0.5, 0.5, 0.9, 0.4);
    int x = rand() % (WIN3_WIDTH - 30);
    int y = rand() % (WIN3_HEIGHT - 20);
    int r = rand() % 30;
    cairo_arc(win, x, y, r, 0, 2 * M_PI);
    cairo_fill(win);
 
    cairo_pop_group_to_source(win);
    cairo_paint(win);
}

void paint4(cairo_t *win)
{
    cairo_push_group(win);

    draw_svg_to_win(win4, 20, 20, "images/toy-robot.svg", 200, 200); 
    cairo_stroke(win);

    cairo_pop_group_to_source(win);
    cairo_paint(win);
}

void key_listener(int win, int key)
{
    int x = x3;
    int y = y3;
    if (key == GUI_ESC_KEY) program_runs = 0;
    else if (key == GUI_RIGHT_ARROW_KEY)
    {
        if (x3 < WIN3_WIDTH - 1) x3++;
        line3_from(x, y);
    }
    else if (key == GUI_LEFT_ARROW_KEY)
    {
        if (x3 > 0) x3--;
        line3_from(x, y);
    }
    else if (key == GUI_UP_ARROW_KEY)
    {
        if (y3 > 0) y3--;
        line3_from(x, y);
    }
    else if (key == GUI_DOWN_ARROW_KEY)
    {
        if (y3 < WIN3_HEIGHT - 1) y3++;
        line3_from(x, y);
    }
    else if ((key != GUI_TAB_KEY) && (key != GUI_SHIFT_TAB_KEY)) printf("key %d\n", key);
}

void test_gui()
{
    win1 = gui_open_window(paint1, WIN1_WIDTH, WIN1_HEIGHT, 300);
    win2 = gui_open_window(paint2, WIN2_WIDTH, WIN2_HEIGHT, 100);
    win3 = gui_open_window(paint3, WIN3_WIDTH, WIN3_HEIGHT, 100);
    win4 = gui_open_window(paint4, WIN4_WIDTH, WIN4_HEIGHT, 200);

    x1 = WIN1_WIDTH / 2;
    yy1 = WIN1_HEIGHT / 2;
    x2 = WIN2_WIDTH / 2;
    y2 = WIN2_HEIGHT / 2;
    x3 = WIN3_WIDTH / 2;
    y3 = WIN3_HEIGHT / 2;

    gui_set_window_title(win1, "click to draw");
    gui_set_window_title(win3, "use arrows to draw");
    gui_set_window_title(win4, "shows svg file");

    gui_add_mouse_listener(win1, mouse1);
    gui_add_mouse_listener(win2, mouse2);
   
    gui_add_key_listener("drawing", key_listener);

    printf("Welcome to test_gui!\n\n");
    printf(" press TAB/SHIFT-TAB to switch between drawing/system contexts.\n");
    printf(" press ESC to quit\n");
    printf(" when in drawing context,\n  click with mouse to draw lines to first window,\n  random lines to second window,\n  and using arrow keys in the third window\n");

    while (program_runs)
    {
      usleep(100000);
    }
}

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_gui();
    test_gui();

    mikes_shutdown();

    return 0;
}

