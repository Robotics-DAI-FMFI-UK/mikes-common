#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "modules/passive/x_xtion.h"
#include "modules/live/gui.h"
#include "bites/mikes.h"

static int width = 640, height = 480;
static double zoom = 2.0;
 
void process_command_line_arguments(int argc, char** argv)
{
    if ((argc > 1) && (isdigit(argv[1][0])))
    {
      sscanf(argv[1], "%d", &width);
      height = (int)(480.0 / 640.0 * width + 0.5);
      if ((argc > 2) && (isdigit(argv[2][0])))
        sscanf(argv[2], "%lf", &zoom);
      if (zoom < 1.0)
      {
        printf("Zoom < 1.0 is not supported.\n");
        exit(1);
      }
      printf("width=%d, height=%d, zoom=%.2lf\n", width, height, zoom);
    }
    else 
    {
      printf("using 640x480, you can specify another width on command line, usage:\n");
      printf("./test_x_xtion [[width] [zoom_factor]]\n");
      printf("\nswitch to saving context with TAB, and press ENTER to save to PNG\n");
    }
}

int main(int argc, char **argv)
{
    process_command_line_arguments(argc, argv);

    mikes_init(argc, argv);

    init_xtion(width, height);
    init_gui();
    init_x_xtion(width, height, zoom, 100);

    sleep(1);

    while (program_runs)
    {
      usleep(30000);
    }

    shutdown_x_xtion();
    shutdown_gui();

    mikes_shutdown();

    return 0;
}
