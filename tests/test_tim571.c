#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "modules/live/tim571.h"
#include "bites/mikes.h"

void test_tim571()
{
    uint16_t buffer[TIM571_DATA_COUNT];
    uint8_t buf2[TIM571_DATA_COUNT];
    char strbuf[3000];
    tim571_status_data status_data;

    get_tim571_dist_data(buffer);
    get_tim571_rssi_data(buf2);
    get_tim571_status_data(&status_data);
    pretty_print_status_data(strbuf, &status_data);
    printf("%s\n", strbuf);
    for (int i = 0; i < status_data.data_count; i++)
    {
            if (i % 10 == 0) printf("\n%3d  ", i);
            printf("%5u (%3u)  ", buffer[i], buf2[i]);
    }
    printf("\n---\n");
}


int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_tim571();
    sleep(1);
    test_tim571();

    mikes_shutdown();

    return 0;
}
