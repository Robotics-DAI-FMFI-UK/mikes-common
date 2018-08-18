#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "modules/passive/x_line_map.h"
#include "modules/live/gui.h"
#include "bites/mikes.h"

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_gui();
    init_x_line_map("images/pavilon_I.svg", 600, 600);

    while (program_runs)
    {
      usleep(30000);
    }

    shutdown_x_line_map();
    shutdown_gui();

    mikes_shutdown();

    return 0;
}
