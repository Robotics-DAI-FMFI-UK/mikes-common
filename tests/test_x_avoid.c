#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "modules/passive/x_tim571.h"
#include "modules/live/gui.h"
#include "modules/live/tim_hough_transform.h"
#include "modules/live/line_filter.h"
#include "modules/live/tim_segment.h"
#include "modules/live/tim_corner.h"
#include "modules/live/base_module.h"
#include "modules/live/avoid.h"
#include "bites/mikes.h"


int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_base_module();

    init_gui();
    init_tim571();
    sleep(1);
    init_tim_hough_transform();
    init_line_filter();
    init_tim_segment();
    init_tim_corner();
    init_x_tim571(7000, 400);
    init_avoid();

    avoid_zone_enable(1, 1);

    while (program_runs)
    {
      for(int i = 0; i < 10*5; i++) { set_motor_speeds(12, 12);  usleep(200000); }
      set_motor_speeds(0, 0);
      sleep(1);
      for(int i = 0; i <  7*5; i++) { set_motor_speeds(12,  0);  usleep(200000); }
      set_motor_speeds(0, 0);
      sleep(1);
    }

    shutdown_avoid();
    shutdown_line_filter();
    shutdown_tim_hough_transform();
    shutdown_x_tim571();
    shutdown_gui();

    mikes_shutdown();

    return 0;
}
