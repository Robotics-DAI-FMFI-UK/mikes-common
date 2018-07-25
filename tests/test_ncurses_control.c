#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../modules/live/ncurses_control.h"
#include "../bites/mikes.h"
#include "core/config_mikes.h"

static int number;
static int win[6];
static char wname[6][30];

static int winkeys;

void counting(int key)
{
    if (key == 'n')
    {
        number++;
        for (int i = 0; i < 6; i++)
            if (number % (i + 2) == 0)
                 print_int_to_window(win[i], number);
    }
    else if (key == ' ')
    {
        for (int i = 0; i < 6; i++)
             println_to_window(win[i], "hello everybody!");
    }
    else if (key == 'r')
    {
        int r = rand() % 6;
        println_to_window(win[r], "hi the random one!");
    }
    else if (key == 'd')
    {
        double d = rand() / (double)RAND_MAX;
        int r = rand() % 6;
        print_double_to_window(win[r], d);
        r = rand() % 6;
        print_doublemsg_to_window(win[r], "rnddouble=", d);
    }
    else if (key == 'i')
    {
        int i = rand();
        int r = rand() % 6;
        print_int_to_window(win[r], i);
        r = rand() % 6;
        print_intmsg_to_window(win[r], "rndint=", i);
    }
    else if (key == KEY_ESC)
        program_runs = 0;
    else print_intmsg_to_window(winkeys, "key: ", key);
}

int main(int argc, char **argv)
{
    number = 1;

    mikes_init(argc, argv);

    init_ncurses_control();

    for (int i = 0; i < 6; i++) 
    {
      win[i] = open_window();
      sprintf(wname[i], "mod %d == 0", i + 2);
      set_window_title(win[i], wname[i]);
    }
    winkeys = open_window();
    set_window_title(winkeys, "keys");
    
    add_key_listener("counting", counting);
    set_current_context("counting");
 
    println_to_window(win[0], "Use keys n,r,d,i, and ' '");

    while (program_runs) usleep(1000);
 
    mikes_shutdown();
  
    return 0;
}
