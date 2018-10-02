#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "modules/passive/x_tim571.h"
#include "modules/live/gui.h"
#include "modules/live/tim_hough_transform.h"
#include "modules/live/line_filter.h"
#include "bites/mikes.h"

void key_listener(int win, int key)
{
    if (key == 'r') show_raw_lines ^= 1;
    if (key == 'f') show_filtered_lines ^= 1;
    if (key == 's') show_segments ^= 1;
}

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_gui();
    init_tim571();
    sleep(1);
    init_tim_hough_transform();
    init_line_filter();
    init_x_tim571(7000, 400);
    gui_add_key_listener("showing", key_listener);

    while (program_runs)
    {
      usleep(30000);
    }

    shutdown_line_filter();
    shutdown_tim_hough_transform();
    shutdown_x_tim571();
    shutdown_gui();

    mikes_shutdown();

    return 0;
}
