#ifndef _GUI_H_
#define _GUI_H_

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>

#define LEFT_MOUSE_BUTTON -1
#define RIGHT_MOUSE_BUTTON -2

#define GUI_ESC_KEY         0xff1b
#define GUI_TAB_KEY         65289
#define GUI_SHIFT_TAB_KEY   65056

#define GUI_RIGHT_ARROW_KEY 0xff53
#define GUI_LEFT_ARROW_KEY  0xff51
#define GUI_UP_ARROW_KEY    0xff52
#define GUI_DOWN_ARROW_KEY  0xff54

typedef void (*gui_key_callback)(int, int);  // active window, key
typedef void (*gui_draw_callback)(cairo_t *);  // window cairo handle     
typedef void (*gui_mouse_callback)(int, int, int);  // x, y, button

void init_gui();
void shutdown_gui();

int gui_open_window(gui_draw_callback paint, int width, int height, int update_period_in_ms);
void gui_close_window(int window_handle);
void gui_set_window_title(int window_handle, char *title);
cairo_t *get_cairo_t(int window_handle);

void gui_add_key_listener(char *context, gui_key_callback callback);
void remove_key_listener(char *context);
void gui_add_mouse_listener(int window_handle, gui_mouse_callback callback);

char *get_current_gui_context();
void set_current_gui_context(char *context);

#endif
