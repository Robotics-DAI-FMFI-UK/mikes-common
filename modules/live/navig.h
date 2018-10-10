#ifndef _NAVIG_H_
#define _NAVIG_H_

#include <pthread.h>
#include "../passive/pose.h"
#include "../live/base_module.h"

#define NAVIG_MAX_CALLBACKS 20

#define NAVIG_CMD_ID_NONE      0

#define NAVIG_RESULT_OK        0
#define NAVIG_RESULT_FAILED   -1
#define NAVIG_RESULT_WAIT      1

typedef struct {
    int navig_cmd_id;
    int navig_result;    /* NAVIG_RESULT_ */
} navig_callback_data_t;

typedef void (*navig_data_callback)(navig_callback_data_t *data);

typedef void (*navig_actualize_pose_function)(void);

#define NAVIG_CMD_ID_NONE            0

#define NAVIG_CMD_TYPE_NONE          0
#define NAVIG_CMD_TYPE_GOTO_POINT    1

#define NAVIG_STATE_NONE             0
#define NAVIG_STATE_WAIT_CMD         1
#define NAVIG_STATE_GOTO_POINT       2
#define NAVIG_STATE_CMD_FINISH       3
#define NAVIG_STATE__COUNT           4  /* count */

typedef struct {
    int init;
    pthread_t thread;

    int data_fd[2];
    pthread_mutex_t data_lock;

    int callbacks_count;
    navig_data_callback callbacks[NAVIG_MAX_CALLBACKS];

    navig_actualize_pose_function update_pose_function;

    base_data_type base_data;
    pose_type      pose_data;

    int state;
    int state_old;
    int cmd_id_last;

    int cmd_id;
    int cmd_type;
    double cmd_px;
    double cmd_py;
    double cmd_ph;

    int new_cmd_id;
    int new_cmd_type;
    double new_cmd_px;
    double new_cmd_py;
    double new_cmd_ph;
} navig_t;


int  init_navig();
void shutdown_navig();

int  navig_register_callback(navig_data_callback callback);
void navig_unregister_callback(navig_data_callback callback);

int register_navig_actualize_pose_function(navig_actualize_pose_function fn);

int navig_cmd_goto_point(double px, double py, double ph);
int navig_cmd_get_result(int cmd_id);

#endif
