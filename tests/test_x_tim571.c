#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "modules/passive/x_tim571.h"
#include "modules/live/gui.h"
#include "bites/mikes.h"

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_gui();
    init_tim571();
    sleep(1);
    init_tim_hough_transform();
    init_x_tim571(7000, 400);

    while (program_runs)
    {
      usleep(30000);
    }

    shutdown_x_tim571();
    shutdown_tim_hough_transform();
    shutdown_gui();

    mikes_shutdown();

    return 0;
}
