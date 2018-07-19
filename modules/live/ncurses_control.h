#ifndef _NCURSES_CONTROL_
#define _NCURSES_CONTROL_

#define KEY_UP 259
#define KEY_DOWN 258
#define KEY_LEFT 260
#define KEY_RIGHT 261
#define KEY_ESC 27

typedef void (*ncurses_control_callback)(int);

void init_ncurses_control();

// returns windows handle or -1 if no more windows are available
int open_window();
void close_window(int window_handle);

void set_window_title(int window_handle, char *title);

void println_to_window(int window_handle, char *message);
void print_int_to_window(int window_handle, int value);
void print_double_to_window(int window_handle, double value);
void print_intmsg_to_window(int window_handle, char *message, int value);
void print_doublemsg_to_window(int window_handle, char *message, int value);

void add_key_listener(char *context, void (callback(int)));
void remove_key_listener(char *context);

#endif

