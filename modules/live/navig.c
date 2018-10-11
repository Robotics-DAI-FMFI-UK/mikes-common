#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "navig.h"
#include "../../bites/mikes.h"
#include "../../bites/util.h"
#include "../../bites/math_2d.h"
#include "../passive/mikes_logs.h"
#include "../passive/pose.h"
#include "../live/base_module.h"
#include "core/config_mikes.h"


static navig_t navig;

// fixme: config to be implemented(?)
int mikes_config_use_navig = 1;

int navig_data_lock()
{
    return pthread_mutex_lock(&navig.data_lock);
}

int navig_data_trylock()
{
    return pthread_mutex_trylock(&navig.data_lock);
}

int navig_data_unlock()
{
    return pthread_mutex_unlock(&navig.data_lock);
}

int navig_data_wait()
{
    if (wait_for_new_data(navig.data_fd) < 0) {
        perror("mikes:navig");
        mikes_log(ML_ERR, "[navig] navig::navig_data_wait(): msg=\"navig error during waiting on new Data.\"");
        return -1;
    }
    return 0;
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// -----------------------ACTUALIZE POSE---------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

#define WAIT_SOME_LOST_MESSAGES_TO_FLUSH 500000

int request_update_actual_pose()
{
  int result = 0;

  navig_data_lock();
  if (navig.update_pose_function) {
    navig.was_updated_localize = 0;
    result = 1;
    navig.update_pose_function(NAVIG_START_LOCALIZE);
  }
  navig_data_unlock();

  return result;
}

int navig_register_actualize_pose_function(navig_actualize_pose_function fn)
{
  if (!navig.init) return -1;

  navig_data_lock();
  navig.update_pose_function = fn;
  navig_data_unlock();

  return 0;
}

int navig_can_actualize_pose_now()
{
  int result = 0;

  navig_data_lock();
  if (navig.update_pose_function && (navig.state == NAVIG_STATE_WAIT_LOCALIZE_BEFORE || navig.state == NAVIG_STATE_WAIT_LOCALIZE_AFTER) && navig.was_updated_localize == 0) {
    navig.was_updated_localize = 1;
    result = 1;
    navig.update_pose_function(NAVIG_STOP_LOCALIZE);
    usleep(WAIT_SOME_LOST_MESSAGES_TO_FLUSH);
  }
  navig_data_unlock();

  return result;
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------LIFECYCLE-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

#define NAVIG_MOVE_SPEED              (12.0)
#define NAVIG_DIST_THRESHOLD          (10.0)  /* in cm */
#define NAVIG_HEADING_THRESHOLD       (10.0 / 180.0 * M_PI)  /* in radians */

#define NAVIG_TURN_SPEED              (12.0)
//#define NAVIG_TURN_DIST_THRESHOLD     (30.0)  /* in cm */
#define NAVIG_TURN_HEADING_THRESHOLD  (30.0 / 180.0 * M_PI)  /* in radians */

#define NAVIG_LOGSTR_LEN   1024

#define NAVIG_ACTION_NONE     0
#define NAVIG_ACTION_MOVE     1
#define NAVIG_ACTION_TURN     2
#define NAVIG_ACTION_STOP     3
#define NAVIG_ACTION_COUNT    4

static char *navig_action_str[NAVIG_ACTION_COUNT] = { "none", "move", "turn", "stop" };

void navig_log_data(int action, double dx, double dy, double dd, double navig_head, double apose_head, double dh,
         int turn_only, double left_motor, double right_motor, int res)
{
    char str[NAVIG_LOGSTR_LEN];

    if ((action < 0) || (action >= NAVIG_ACTION_COUNT)) action = NAVIG_ACTION_NONE;

    sprintf(str, "[navig] navig::navig_log_data(): action=\"%s\", dx=%0.2f, dy=%0.2f, dd=%0.2f, navig_head_deg=%0.2f, apose_head_deg=%0.2f, dh_deg=%0.2f, turn_only=%d, left_motor=%0.2f, right_motor=%0.2f, res=%d",
        navig_action_str[action], dx, dy, dd, navig_head / M_PI * 180.0, apose_head / M_PI * 180.0, dh  / M_PI * 180.0,
        turn_only, left_motor, right_motor, res);

    mikes_log(ML_DEBUG, str);
}

int navig_to_point(double px, double py, double ph)
// returns: 0 = wait navigating to the point; +1 = navigation point reached (stopping)
{
    double left_motor  = 0;
    double right_motor = 0;

    pose_type apose;
    get_pose(&apose);

    vector_2d d;
    d.x = px - apose.x;
    d.y = py - apose.y;

    double dd = get_vector_length(&d);

    double navig_head = 0;

    int turn_only = 0;

    /* compare distance to navigation point */
    if (dd < NAVIG_DIST_THRESHOLD) {
        navig_head = ph;
        turn_only = 1;
    } else {
        // angle from x axis (counterclockwise), in radians
        double ang = angle_from_axis_x(&d) * M_PI / 180.0;

        // heading from y axis (clockwise), in radians
        navig_head = M_PI / 2.0 - ang;
    }

    /* dh: relative heading towards navigation point (-M_PI, +M_PI) */
    double dh = navig_head - apose.heading;
    if (dh < -M_PI) dh += 2 * M_PI;
    if (dh > +M_PI) dh -= 2 * M_PI;

    /* stop if we are close with correct heading */
    if ((dh >= -NAVIG_HEADING_THRESHOLD) && (dh <= NAVIG_HEADING_THRESHOLD)) {
        if (turn_only) {
            stop_now();
            navig_log_data(NAVIG_ACTION_STOP, d.x, d.y, dd, navig_head, apose.heading, dh, turn_only, left_motor, right_motor, +1 /*res*/);
            return +1;  /* navigation point reached (stopping) */
        }
    }

//  /* if we are too close use turn only */
//  if (dd < NAVIG_TURN_DIST_THRESHOLD) {
//      turn_only = 1;  // fixme
//  }

    /* turn to final direction */
    if ((dh < -NAVIG_TURN_HEADING_THRESHOLD) || (dh > NAVIG_TURN_HEADING_THRESHOLD) || turn_only) {
        if (dh >= 0) {
            left_motor  = NAVIG_TURN_SPEED;
            right_motor = -NAVIG_TURN_SPEED;
        } else {
            left_motor  = -NAVIG_TURN_SPEED;
            right_motor = NAVIG_TURN_SPEED;
        }
        set_motor_speeds((int)left_motor, (int)right_motor);
        navig_log_data(NAVIG_ACTION_TURN, d.x, d.y, dd, navig_head, apose.heading, dh, turn_only, left_motor, right_motor, 0 /*res*/);
        return 0;
    }

    /* move forward/left/right */
    left_motor  = NAVIG_MOVE_SPEED;
    right_motor = NAVIG_MOVE_SPEED;

    /* relative difference between motor speeds */
    double diff = dh / (M_PI / 4);

    if (diff > 0.5) {
        diff = 0.5;
    } else
    if (diff < -0.5) {
        diff = -0.5;
    }

    if (diff >= 0) {
        right_motor *= (1 - diff);
    } else {
        left_motor *= (1 + diff);
    }

    set_motor_speeds((int)left_motor, (int)right_motor);
    navig_log_data(NAVIG_ACTION_MOVE, d.x, d.y, dd, navig_head, apose.heading, dh, turn_only, left_motor, right_motor, 0 /*res*/);

    return 0;
}

void navig_read_data()
{
    get_base_data(&navig.base_data);
    log_base_data(&navig.base_data);

    get_pose(&navig.pose_data);
    log_pose(&navig.pose_data);
}

static char *navig_state_str[NAVIG_STATE__COUNT] = { "none", "wait_cmd", "goto_point", "cmd_finish", "request_localize_before", "wait_localize_before", "request_localize_after", "wait_localize_after" };

void navig_log_process_state(int state, int state_old)
{
    char str[NAVIG_LOGSTR_LEN];

    /* log only state changes */
    if (state == state_old) return;

    if ((state < 0) || (state >= NAVIG_STATE__COUNT)) state = NAVIG_STATE_NONE;
    if ((state_old < 0) || (state_old >= NAVIG_STATE__COUNT)) state_old = NAVIG_STATE_NONE;

    sprintf(str, "[navig] navig::navig_log_process_state(): msg=\"state changed!\", navig_state=\"%s\", navig_state_old=\"%s\"",
        navig_state_str[state], navig_state_str[state_old]);

    mikes_log(ML_DEBUG, str);
}


void navig_process_data()
{
    navig_log_process_state(navig.state, navig.state_old);
    navig.state_old = navig.state;

    switch (navig.state) {
        case NAVIG_STATE_WAIT_CMD:
            if ((navig.new_cmd_id != NAVIG_CMD_ID_NONE) &&
                (navig.new_cmd_type == NAVIG_CMD_TYPE_GOTO_POINT)) {
                navig.cmd_id = navig.new_cmd_id;
                navig.cmd_px = navig.new_cmd_px;
                navig.cmd_py = navig.new_cmd_py;
                navig.cmd_ph = navig.new_cmd_ph;

                navig.new_cmd_id = NAVIG_CMD_ID_NONE;
                navig.state = NAVIG_STATE_REQUEST_LOCALIZE_BEFORE;
            }
            break;

        case NAVIG_STATE_REQUEST_LOCALIZE_BEFORE:
            if (request_update_actual_pose()) {
                navig.state = NAVIG_STATE_WAIT_LOCALIZE_BEFORE;
            } else {
                navig.state = NAVIG_STATE_GOTO_POINT;
            }
            break;

        case NAVIG_STATE_WAIT_LOCALIZE_BEFORE:
            if (navig.was_updated_localize) {
              navig.state = NAVIG_STATE_GOTO_POINT;
            }
            break;

        case NAVIG_STATE_GOTO_POINT:
            if (navig_to_point(navig.cmd_px, navig.cmd_py, navig.cmd_ph) > 0) {
                navig.state = NAVIG_STATE_REQUEST_LOCALIZE_AFTER;
            }
            break;

        case NAVIG_STATE_REQUEST_LOCALIZE_AFTER:
            if (request_update_actual_pose()) {
                navig.state = NAVIG_STATE_WAIT_LOCALIZE_AFTER;
            } else {
                navig.state = NAVIG_STATE_CMD_FINISH;
            }
            break;

        case NAVIG_STATE_WAIT_LOCALIZE_AFTER:
            if (navig.was_updated_localize) {
              navig.state = NAVIG_STATE_CMD_FINISH;
            }
            break;

        case NAVIG_STATE_CMD_FINISH: {
            navig_callback_data_t callback_data;

            callback_data.navig_cmd_id = navig.cmd_id;
            callback_data.navig_result = NAVIG_RESULT_OK;

            for (int i = 0; i < navig.callbacks_count; i++) {
                navig.callbacks[i](&callback_data);
            }

            navig.cmd_id = NAVIG_CMD_ID_NONE;
            navig.cmd_type = NAVIG_CMD_TYPE_NONE;
            navig.state = NAVIG_STATE_WAIT_CMD;
            break;
           }
    }
}

int navig_cmd_goto_point(double px, double py, double ph)
// returns cmd_id
{
    navig_data_lock();

    int new_cmd_id = (++navig.cmd_id_last);

    navig.new_cmd_id = new_cmd_id;
    navig.new_cmd_type = NAVIG_CMD_TYPE_GOTO_POINT;
    navig.new_cmd_px = px;
    navig.new_cmd_py = py;
    navig.new_cmd_ph = ph;

    navig_data_unlock();

    char str[NAVIG_LOGSTR_LEN];

    sprintf(str, "[main] navig::navig_cmd_goto_point(): cmd_id=%d, cmd_type=%d, px=%0.2f, py=%0.2f, ph_deg=%0.2f",
        new_cmd_id, NAVIG_CMD_TYPE_GOTO_POINT, px, py, ph / M_PI * 180.0);

    mikes_log(ML_INFO, str);

    return new_cmd_id;
}

int navig_cmd_get_result(int cmd_id)
{
    navig_data_lock();

    // fixme
    int res = NAVIG_RESULT_OK;

    if (navig.cmd_id == cmd_id) {
       res = NAVIG_RESULT_WAIT;
    }

    navig_data_unlock();

    if (res != NAVIG_RESULT_WAIT) {
        char str[NAVIG_LOGSTR_LEN];
        sprintf(str, "[main] navig::navig_cmd_get_result(): cmd_id=%d, res=%d", cmd_id, res);
        mikes_log(ML_INFO, str);
    }

    return res;
}

void *navig_thread(void *args)
{
    mikes_log(ML_INFO, "[navig] navig::navig_thread(): msg=\"navig starts.\"");

    while (program_runs && !navig.terminate) {
        // fixme: not using calbacks currently
        //navig_data_wait();

        navig_data_lock();
        navig_read_data();

        navig_process_data();
        navig_data_unlock();

        // fixme: how often should we write commands to base_module (50x per second)
        usleep(20000);
    }

    mikes_log(ML_INFO, "[navig] navig::navig_thread(): msg=\"navig quits.\"");
    threads_running_add(-1);
    return 0;
}

void navig_new_base_data(base_data_type *data)
{
    if (navig_data_trylock() == 0) {
        navig.base_data = *data;
        alert_new_data(navig.data_fd);
        navig_data_unlock();
    }
}

int navig_init()
{
    navig.init = 0;
    navig.terminate = 0;
    navig.callbacks_count = 0;
    navig.was_updated_localize = 0;
    navig.update_pose_function = 0;
    memset(&navig.base_data, 0, sizeof(navig.base_data));
    memset(&navig.pose_data, 0, sizeof(navig.pose_data));

    navig.state = NAVIG_STATE_WAIT_CMD;
    navig.state_old = NAVIG_STATE_NONE;
    navig.cmd_id_last = NAVIG_CMD_ID_NONE;

    navig.cmd_id = NAVIG_CMD_ID_NONE;
    navig.cmd_type = NAVIG_CMD_TYPE_NONE;
    navig.cmd_px = 0;
    navig.cmd_py = 0;
    navig.cmd_ph = 0;

    navig.new_cmd_id = NAVIG_CMD_ID_NONE;
    navig.new_cmd_type = NAVIG_CMD_TYPE_NONE;
    navig.new_cmd_px = 0;
    navig.new_cmd_py = 0;
    navig.new_cmd_ph = 0;

    if (!mikes_config_use_navig)
    {
        mikes_log(ML_INFO, "[main] navig::navig_init(): msg=\"navig supressed by config.\"");
        return 0;  //fixme
    }

    if (pipe(navig.data_fd) != 0)
    {
        perror("mikes:navig");
        mikes_log(ML_ERR, "[main] navig::navig_init(): msg=\"error creating pipe!\"");
        return -2;
    }

    if (pthread_mutex_init(&navig.data_lock, 0) != 0)
    {
        perror("mikes:navig");
        mikes_log(ML_ERR, "[main] navig::navig_init(): msg=\"error creating mutex!\"");
        return -3;
    }

    register_base_callback(navig_new_base_data);

    if (pthread_create(&navig.thread, 0, navig_thread, 0) != 0)
    {
        perror("mikes:navig");
        mikes_log(ML_ERR, "[main] navig::navig_init(): msg=\"error creating thread!\"");
        return -4;
    }

    threads_running_add(1);
    navig.init = 1;

    return 0;
}

void navig_close()
// call navig_stop() before navig_shutdown
{
    if (!navig.init) return;

    unregister_base_callback(navig_new_base_data);

    if (pthread_mutex_destroy(&navig.data_lock) != 0) {
        mikes_log(ML_ERR, "[main] navig::navig_shutdown(): msg=\"error destroying mutex!\"");
    }

    close(navig.data_fd[0]);
    close(navig.data_fd[1]);

    navig.init = 0;
}

int navig_stop(void)
{
    if (!navig.init) return -1;

    if (program_runs) {
        mikes_log(ML_ERR, "[main] navig::navig_stop(): msg=\"wrong program_runs value - unable to stop!\"");
        return -2;
    }

    mikes_log(ML_INFO, "[main] navig::navig_stop(): msg=\"joining threads...\"");

    void *status;
    navig.terminate = 1;
    pthread_join(navig.thread, &status);

    mikes_log(ML_INFO, "[main] navig::navig_stop(): msg=\"finished\"");
    return 0;
}

int init_navig()
{
    return navig_init();
}

void shutdown_navig()
{
    navig_stop();
    navig_close();
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------CALLBACK------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

int navig_register_callback(navig_data_callback callback)
{
    if (!navig.init) return -1;

    if (navig.callbacks_count >= NAVIG_MAX_CALLBACKS)
    {
         mikes_log(ML_ERR, "[main] navig::navig_register_callback(): msg=\"too many navig callbacks\"");
         return -2;
    }
    navig.callbacks[navig.callbacks_count++] = callback;
    return 0;
}

void navig_unregister_callback(navig_data_callback callback)
{
    if (!navig.init) return;

    for (int i = 0; i < navig.callbacks_count; i++) {
        if (navig.callbacks[i] == callback) {
            navig.callbacks[i] = navig.callbacks[--navig.callbacks_count];
        }
    }
}
