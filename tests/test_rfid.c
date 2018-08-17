#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../modules/live/rfid_sensor.h"
#include "../modules/passive/mikes_logs.h"
#include "../bites/mikes.h"
#include "../core/config_mikes.h"

void test_rfid_sensor()
{
    rfid_data_type rfid_data;
    while (1)
    {
    	get_rfid_data(&rfid_data);
        for (int i = 0; i < rfid_data.ntags; i++)
           printf("(x=%d, y=%d, a=%d)\n", rfid_data.x[i], rfid_data.y[i], rfid_data.a[i]);
        printf("---\n");
        usleep(200000);
    }
}

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_rfid_sensor();
    test_rfid_sensor();

    mikes_shutdown();
}

