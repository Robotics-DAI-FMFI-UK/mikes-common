#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>

#include "../../bites/mikes.h"
#include "../passive/mikes_logs.h"
#include "core/config_mikes.h"
#include "../../bites/util.h"
#include "ncurses_control.h"

#define KEY_TAB             9
#define MAX_CONTEXTS_COUNT 10

static WINDOW *mainwin;

char *context_names[MAX_CONTEXTS_COUNT];
ncurses_control_callback callbacks[MAX_CONTEXTS_COUNT];
volatile int contexts_count;
volatile int current_context;

void add_key_listener(char *context, void (callback(int)))
{
  if (contexts_count == MAX_CONTEXTS_COUNT)
  {
    mvprintw(0, 60, "contexts full!");
    return;
  }
  context_names[contexts_count] = context;
  callbacks[contexts_count] = callback;
  contexts_count++;
}

void remove_key_listener(char *context)
{
}

void print_current_context()
{
    mvprintw(0, 43, context_names[current_context]);
    refresh();
}

void *ncurses_control_thread(void *arg)
{
    int ch;
    if ((mainwin = initscr()) == 0)
    {
      mikes_log(ML_WARN, "Error initializing ncurses, not using ncurses");
      return 0;
    }

    noecho();                  /*  Turn off key echoing                 */
    keypad(mainwin, TRUE);     /*  Enable the keypad for non-char keys  */


    /*  Print a prompt and refresh() the screen  */

    mvaddstr(0, 0, "Mikes");
//Control Kocur Mikes with arrow keys, space to backup, 'a' to toggle autopilot, ESC to quit");
    mvprintw(0, 10, "TAB switches context. Context: ");
    for (int i = 0; i < 80; i++) mvprintw(1, i, "-");
    print_current_context();

    while (program_runs)
    {
        ch = getch();
        if (ch == KEY_TAB) 
        {
          current_context++;
          if (current_context == contexts_count) current_context = 0;
          print_current_context();
        } 
        else callbacks[current_context](ch);
    }
            
    delwin(mainwin);
    endwin();
    refresh();

    mikes_log(ML_INFO, "ncurses quits.");
    threads_running_add(-1);

    return 0;
}

void system_context_callback(int key)
{
    switch(key) {
       case KEY_ESC: program_runs = 0;
                     mikes_log(ML_INFO, "quit by ESC");
    }
}

void init_ncurses_control()
{
    contexts_count = 1;
    current_context = 0;
    context_names[0] = "system";
    callbacks[0] = system_context_callback;    

    if (!mikes_config.use_ncurses_control) return;

    pthread_t t;
    if (pthread_create(&t, 0, ncurses_control_thread, 0) != 0)
    {
        perror("mikes:ncurses");
        mikes_log(ML_ERR, "creating ncurses thread");
    }
    else threads_running_add(1);
}


int open_window()
{
  return 0;
}

void close_window(int window_handle)
{
}

void set_window_title(int window_handle, char *title)
{
}


void println_to_window(int window_handle, char *message)
{
    mvprintw(3, 0, message);
}

void print_int_to_window(int window_handle, int value)
{
}

void print_double_to_window(int window_handle, double value)
{
}

void print_intmsg_to_window(int window_handle, char *message, int value)
{
}

void print_doublemsg_to_window(int window_handle, char *message, int value)
{
}

