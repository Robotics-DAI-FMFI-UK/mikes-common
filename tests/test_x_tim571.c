#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "modules/passive/x_tim571.h"
#include "modules/live/gui.h"
#include "modules/live/tim_hough_transform.h"
#include "modules/live/line_filter.h"
#include "modules/live/tim_segment.h"
#include "bites/mikes.h"


int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_gui();
    init_tim571();
    sleep(1);
    init_tim_hough_transform();
    init_line_filter();
    init_tim_segment();
    init_x_tim571(7000, 400);

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
