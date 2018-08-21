#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "modules/passive/x_line_map.h"
#include "modules/live/gui.h"
#include "bites/mikes.h"
#include "modules/passive/mikes_logs.h"
#include "core/config_mikes.h"

static pose_type p;

void animate_pose()
{
    x_line_map_set_pose(p);
    x_line_map_toggle_pose_visible(1);

    for (int way = 0; way < 2; way++)
    {
      for (int i = 0 ; i < 3000; i += 20)
      {
         p.x = (way == 0)?(4400 + i):(7400-i);

         int rndsign = (2 * (rand() % 2) - 1);
         double delta = (rand() / (double)RAND_MAX) * 10 * rndsign;
         p.y += delta;

         x_line_map_set_pose(p);
         usleep(20000);
         if (!program_runs) return;
      }
  
      for (int i = 0 ; i < 180; i += 5)
      {
        p.heading += 5.0 * M_PI / 180.0;
        x_line_map_set_pose(p);
        usleep(20000);
        if (!program_runs) return;
      }
    }
    x_line_map_toggle_pose_visible(0);
}

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_gui();
    init_x_line_map(mikes_config.line_map_file, 600, 600);

    p.x = 4400;
    p.y = 36500;
    p.heading = M_PI / 2;

    while (program_runs)
    {
      animate_pose(); 
      usleep(30000);
    }

    shutdown_x_line_map();
    shutdown_gui();

    mikes_shutdown();

    return 0;
}

