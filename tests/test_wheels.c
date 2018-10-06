#include <stdio.h>
#include <unistd.h>

#include "../modules/passive/wheels.h"
#include "../bites/mikes.h"

void test_wheels()
{
    printf("test\n");
    wheels_test();
    sleep(4);
    printf("up\n");
    wheels_up();
    sleep(2);
    printf("down\n");
    wheels_down();
    sleep(2);
    printf("fwd\n");
    wheels_forward();
    sleep(2);
    printf("up\n");
    wheels_up();
    printf("bye\n");
}

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_wheels();
    test_wheels();

    mikes_shutdown();
}

