#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include "modules/passive/x_base.h"
#include "modules/live/gui.h"
#include "modules/live/base_module.h"
#include "bites/mikes.h"

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_gui();
    init_base_module();
    init_x_base(400);

    while (program_runs)
    {
      usleep(30000);
      if (rand() % 300 == 0) x_base_set_azimuth(rand() % 360);
    }

    shutdown_x_base();
    shutdown_gui();

    mikes_shutdown();

    return 0;
}
