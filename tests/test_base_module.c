#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../modules/live/base_module.h"
#include "../modules/passive/pose.h"
#include "../modules/passive/mikes_logs.h"
#include "../bites/mikes.h"
#include "../core/config_mikes.h"

#define TEST_LENGTH 18

int times[TEST_LENGTH] = {  1000,  3000,
                            8000,  9000, 11800, 12800,
                           17800, 18800, 21600, 22600,
                           27600, 28600, 31400, 32400,
                           37400, 38400, 41200, 42200
                         };
int rspeed[TEST_LENGTH] = { 0, 12, 0, 12,  0, 12, 0, 12, 0, 12, 0, 12, 0, 12, 0, 12, 0, 100 };
int lspeed[TEST_LENGTH] = { 0, 12, 0,  0,  0, 12, 0,  0, 0, 12, 0,  0, 0, 12, 0,  0, 0, 100 };

void test_base_module()
{
    base_data_type base_data;
    int tm = 0;
    int tmptr = 0;
    while (1)
    {
    	get_base_data(&base_data);
        printf("%lu: LCNT:%ld RCNT:%ld LVEL:%d RVEL:%d IR:(%d,%d,%d,%d) HEAD:%d ACC:(%d,%d,%d) GYR:(%d,%d,%d)\n",
               base_data.timestamp, base_data.counterA, base_data.counterB, base_data.velocityA, base_data.velocityB,
               base_data.dist1, base_data.dist2, base_data.dist3, base_data.cube, base_data.heading,
               base_data.ax, base_data.ay, base_data.az, base_data.gx, base_data.gy, base_data.gz);

    	log_base_data(&base_data);

	pose_type apose;
	get_pose(&apose);
    	log_pose(&apose);

        if ((tmptr < TEST_LENGTH) && (tm > times[tmptr]))
        {
            if (lspeed[tmptr] == 100) break;
            printf("moving %d %d...\n", lspeed[tmptr], rspeed[tmptr]);
            set_motor_speeds(lspeed[tmptr], rspeed[tmptr]);
            tmptr++;
        }
        usleep(200000);
        tm += 200;
    }
}

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_pose(0, 0);
    init_base_module();

    test_base_module();

    mikes_shutdown();
}

