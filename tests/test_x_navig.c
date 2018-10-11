#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "modules/passive/x_line_map.h"
#include "modules/live/gui.h"
#include "bites/mikes.h"
#include "modules/passive/mikes_logs.h"
#include "core/config_mikes.h"
#include "modules/live/base_module.h"
#include "modules/passive/pose.h"
#include "bites/math_2d.h"
#include "modules/live/navig.h"

#define PROCESS_STATE_NONE    0
#define PROCESS_STATE_INIT    1
#define PROCESS_STATE_POINT1  2
#define PROCESS_STATE_POINT2  3
#define PROCESS_STATE_POINT3  4
#define PROCESS_STATE_POINT1_WAIT  5
#define PROCESS_STATE_POINT2_WAIT  6
#define PROCESS_STATE_POINT3_WAIT  7
#define PROCESS_STATE_EXIT    9
#define PROCESS_STATE_COUNT  10

int proc_state     = PROCESS_STATE_INIT;
int proc_state_old = PROCESS_STATE_NONE;

#define PROCESS_LOGSTR_LEN   1024

static char *processg_state_str[PROCESS_STATE_COUNT] = 
                { "none", "init", "point1", "point2", "point3", "point1_wait", "point2_wait", "point3_wait", "?8", "exit" };

void log_proc_state(int state, int state_old)
{
    char str[PROCESS_LOGSTR_LEN];

    /* log only state changes */
    if (state == state_old) return;

    if ((state < 0) || (state >= PROCESS_STATE_COUNT)) state = PROCESS_STATE_NONE;
    if ((state_old < 0) || (state_old >= PROCESS_STATE_COUNT)) state_old = PROCESS_STATE_NONE;

    sprintf(str, "[main] process::log_proc_state(): msg=\"state changed!\", proc_state=\"%s\", proc_state_old=\"%s\"",
        processg_state_str[state], processg_state_str[state_old]);

    mikes_log(ML_DEBUG, str);
}

int process_cmd_id = 0;

int process_state(void)
{
    int res = 0;

    log_proc_state(proc_state, proc_state_old);

    proc_state_old = proc_state;

    switch (proc_state) {
        case PROCESS_STATE_INIT: 
            proc_state = PROCESS_STATE_POINT1;
            break;

        case PROCESS_STATE_POINT1: 
            process_cmd_id = navig_cmd_goto_point(303 - 88, 174 - 88, M_PI / 2);
            proc_state = PROCESS_STATE_POINT1_WAIT;
            break;
        case PROCESS_STATE_POINT1_WAIT:
            if (navig_cmd_get_result(process_cmd_id) != NAVIG_RESULT_WAIT) {
                proc_state = PROCESS_STATE_POINT2;
            }
            break;

        case PROCESS_STATE_POINT2: 
            process_cmd_id = navig_cmd_goto_point(303 + 88, 174 + 88, 3 * M_PI / 4);
            proc_state = PROCESS_STATE_POINT2_WAIT;
            break;
        case PROCESS_STATE_POINT2_WAIT:
            if (navig_cmd_get_result(process_cmd_id) != NAVIG_RESULT_WAIT) {
                proc_state = PROCESS_STATE_POINT3;
            }
            break;

        case PROCESS_STATE_POINT3: 
            process_cmd_id = navig_cmd_goto_point(11 + 50, 174, M_PI / 2);
            proc_state = PROCESS_STATE_POINT3_WAIT;
            break;
        case PROCESS_STATE_POINT3_WAIT: 
            if (navig_cmd_get_result(process_cmd_id) != NAVIG_RESULT_WAIT) {
                proc_state = PROCESS_STATE_EXIT;
            }
            break;

        case PROCESS_STATE_EXIT: 
            res = 1;
            stop_now();
            break;
    }

    return res;
}

void test_navig(double px, double py, double heading)
{
    base_data_type base_data;

    set_pose(px, py, heading);  // 10.0

    x_line_map_toggle_pose_visible(1);

    while (1)
    {
    	get_base_data(&base_data);
    	//log_base_data(&base_data);

	pose_type apose;
	get_pose(&apose);
    	//log_pose(&apose);

        x_line_map_set_pose(apose);

        if (process_state() > 0) break; 

        usleep(200000);
    }

    x_line_map_toggle_pose_visible(0);
}

void new_navig_data(navig_callback_data_t *data)
{
    char str[PROCESS_LOGSTR_LEN];

    sprintf(str, "[main] test_x_navig::new_navig_data(): navig_cmd_id=%d, navig_result=%d",
        data->navig_cmd_id, data->navig_result);

    mikes_log(ML_INFO, str);
}

int main(int argc, char **argv)
{
    mikes_init(argc, argv);

    init_pose(0, 0);
    init_base_module();

    init_gui();
    init_x_line_map(mikes_config.line_map_file, 600, 350);

    init_navig();

    navig_register_callback(new_navig_data);

    /* 1 meter from top left corner */
    // test_base_module(11 + 100, 339 - 100, M_PI / 2);
    // test_navig(11 + 100, 339 - 100, M_PI / 2);
    test_navig(11 + 50, 174, M_PI / 2);

    navig_unregister_callback(new_navig_data);

    shutdown_navig();

    shutdown_x_line_map();
    shutdown_gui();

    mikes_shutdown();

    return 0;
}
