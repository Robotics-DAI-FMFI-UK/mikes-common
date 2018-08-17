#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "modules/live/lidar.h"
#include "bites/mikes.h"

void test_lidar()
{
    lidar_data_type data;

    get_lidar_data(&data);
    for (int i = 0; i < data.count; i++)
    {
            
            if (i % 10 == 0) printf("\n%3d", i);
            printf("%6u (%3d,%3d)", data.distance[i], data.angle[i] / 64, data.quality[i]);
    }
    printf("\n---\n");
}


int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_lidar();
    sleep(1);
    test_lidar();

    mikes_shutdown();

    return 0;
}
