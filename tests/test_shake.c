#include <stdio.h>
#include <unistd.h>

#include "../modules/live/base_module.h"
#include "../modules/passive/pose.h"
#include "../modules/passive/mikes_logs.h"
#include "../bites/mikes.h"
#include "../core/config_mikes.h"

#define TEST_LENGTH 18

void test_unloading_shake()
{
    sleep(2);
    printf("shake\n");
    unloading_shake();
}

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_pose(0, 0);
    init_base_module();

    test_unloading_shake();

    mikes_shutdown();
}

