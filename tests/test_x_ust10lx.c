#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "modules/passive/x_ust10lx.h"
#include "modules/live/gui.h"
#include "bites/mikes.h"

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_gui();
    init_ust10lx();
    sleep(1);
    init_x_ust10lx(7000, 400);

    while (program_runs)
    {
      usleep(30000);
    }

    shutdown_x_ust10lx();
    shutdown_gui();

    mikes_shutdown();

    return 0;
}
