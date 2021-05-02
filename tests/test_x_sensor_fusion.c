#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "modules/passive/x_sensor_fusion.h"
#include "modules/live/tim571.h"
#include "modules/live/hcsr04.h"
#include "modules/live/gui.h"
#include "bites/mikes.h"

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_gui();
    init_tim571();
    init_hcsr04();
    sleep(1);
    init_x_sensor_fusion(20000, 400);

    while (program_runs)
    {
      usleep(30000);
    }
	
	shutdown_hcsr04();
    shutdown_x_sensor_fusion();
    shutdown_gui();

    mikes_shutdown();

    return 0;
}
