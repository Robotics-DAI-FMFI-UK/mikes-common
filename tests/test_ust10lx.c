#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "modules/live/ust10lx.h"
#include "bites/mikes.h"

void test_ust10lx()
{
    uint16_t buffer[UST10LX_DATA_COUNT];

    get_ust10lx_data(buffer);
    for (int i = 0; i < UST10LX_DATA_COUNT; i++)
    {
            if (i % 16 == 0) printf("\n%3d  ", i);
            printf("%5u ", buffer[i]);
    }
    printf("\n---\n");
}


int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_ust10lx();
    sleep(1);
    test_ust10lx();

    mikes_shutdown();

    return 0;
}
