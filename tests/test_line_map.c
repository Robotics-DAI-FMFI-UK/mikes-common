#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#include "../modules/passive/line_map.h"
#include "../modules/passive/mikes_logs.h"
#include "../bites/mikes.h"
#include "../core/config_mikes.h"

void test_line_map()
{
  time_t tm;
  time(&tm);
  srand(tm);

  for (int i = 0; i < 50; i++)
  {
     // assume map 45x45 m
     double x = (rand() / (double)RAND_MAX) * 45000.0;
     double y = (rand() / (double)RAND_MAX) * 45000.0;
     double alpha = (rand() / (double)RAND_MAX) * 2 * M_PI;
     double length = 6000;

     double int_x, int_y;

     double dist = find_intersection_of_line_with_line_map(x, y, alpha, length, &int_x, &int_y);
     printf(" [%.3lf,%.3lf] -> %.3lf ...... ", x, y, alpha);
     if (dist < 0) printf("no\n");
     else printf("[%.3lf,%.3lf] at d=%.3lf\n", int_x, int_y, dist);
  }
}

int main(int argc, char **argv)
{
    mikes_init(argc, argv);
    init_line_map("images/pavilon_I.svg");
    test_line_map();
    mikes_shutdown();
}

