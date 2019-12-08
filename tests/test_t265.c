#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "modules/live/t265.h"
#include "bites/mikes.h"

void test_t265()
{
    t265_pose_type t265_pose;
    printf("running for 30s...\n");

    for (int i = 0; i < 30; i++)
    {
      get_t265_pose(&t265_pose);
      log_t265_pose(&t265_pose);
      sleep(1);
    }
    program_runs = 0;
}


int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_t265();
    sleep(1);
    test_t265();

    mikes_shutdown();

    return 0;
}
