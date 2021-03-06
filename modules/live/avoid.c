#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "avoid.h"
#include "../../bites/mikes.h"
#include "../../bites/util.h"
#include "../passive/mikes_logs.h"
#include "../live/base_module.h"
#include "../passive/wheels.h"
#include "core/config_mikes.h"


static avoid_t avoid;

// fixme: config to be implemented(?)
int mikes_config_use_avoid = 1;

int avoid_data_lock()
{
    return pthread_mutex_lock(&avoid.data_lock);
}

int avoid_data_trylock()
{
    return pthread_mutex_trylock(&avoid.data_lock);
}

int avoid_data_unlock()
{
    return pthread_mutex_unlock(&avoid.data_lock);
}

int avoid_data_wait() 
{
    if (wait_for_new_data(avoid.data_fd) < 0) {
        perror("mikes:avoid");
        mikes_log(ML_ERR, "[avoid] avoid::avoid_data_wait(): msg=\"avoid error during waiting on new Data.\"");
        return -1;
    }
    return 0;
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------LIFECYCLE-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------


#define AVOID_LOGSTR_LEN   1024

static char *avoid_state_str[AVOID_STATE__COUNT] = { "none", "stopped", "unblocked" };

int xzone1_cnt, xzone2_cnt, xzone3_cnt;

void avoid_log_data(int state, int state_old, int force)
// force = force logging even if state did not change
{
    char str[AVOID_LOGSTR_LEN];

    /* log only state changes */
    if ((state == state_old) && !force) return;

    if ((state < 0) || (state >= AVOID_STATE__COUNT)) state = AVOID_STATE_NONE;
    if ((state_old < 0) || (state_old >= AVOID_STATE__COUNT)) state_old = AVOID_STATE_NONE;

    sprintf(str, "[avoid] avoid::avoid_log_data(): state=\"%s\", state_old=\"%s\", active=\"%d%d%d\", enabled=\"%d%d%d\", cnt=\"%d,%d,%d\"", 
        avoid_state_str[state], avoid_state_str[state_old], 
        avoid.zone1_active,  avoid.zone2_active,  avoid.zone3_active,
        avoid.zone1_enabled, avoid.zone2_enabled, avoid.zone3_enabled,
        xzone1_cnt, xzone2_cnt, xzone3_cnt);

    mikes_log(ML_DEBUG, str);

   if (state != state_old) {
       log_tim571_data(&avoid.tim571_status, avoid.tim571_dist, avoid.tim571_rssi);
   }
}

int avoid_get_zone_mask(void)
{
    return (avoid.zone1_enabled?0x10:0) | (avoid.zone2_enabled?0x20:0) | (avoid.zone3_enabled?0x40:0) |
           (avoid.zone1_active?0x01:0)  | (avoid.zone2_active?0x02:0)  | (avoid.zone3_active?0x04:0);
}

#define AVOID_TIM571_DISTANCE_BAD            0
#define AVOID_TIM571_RSSI_BAD               100
#define AVOID_TIM571_DISTANCE_MAX            TIM571_MAX_DISTANCE  /* 15000 */
#define AVOID_TIM571_DISTANCE_STEP          20
#define AVOID_TIM571_ANGLE_STEP              1
#define AVOID_TIM571_ANGLE_MULTIPLIER    10000

#define AVOID_ZONE1_ANGLE_MIN             -75
#define AVOID_ZONE1_ANGLE_MAX             +75
#define AVOID_ZONE1_DISTANCE              500    /* mm */
#define AVOID_ZONE1_DATA_COUNT              2

#define AVOID_ZONE2_ANGLE_MIN             -75
#define AVOID_ZONE2_ANGLE_MAX             +75
#define AVOID_ZONE2_DISTANCE              300    /* mm */
#define AVOID_ZONE2_DATA_COUNT              2

#define AVOID_ZONE3_ANGLE_MIN             -75
#define AVOID_ZONE3_ANGLE_MAX             +75
#define AVOID_ZONE3_DISTANCE              100    /* mm */
#define AVOID_ZONE3_DATA_COUNT              2

/*
  tim571_status:
    multiplier = 1.000;
    starting_angle = -450000;
    angular_step = 3333;
    data_count = 811;
    rssi_available = 1;
*/

void avoid_process_data()
{
    avoid_callback_data_t callback_data;

    int mask_old = avoid_get_zone_mask();

    int zone1_cnt = 0;
    int zone2_cnt = 0;
    int zone3_cnt = 0;

    for(int index = 0; index < avoid.tim571_status.data_count; index++) {
        if (avoid.tim571_dist[index] < 30) continue; 
//&&       (avoid.tim571_rssi[index] >= 100)) continue;

        if (avoid.tim571_rssi[index] < AVOID_TIM571_RSSI_BAD) continue;

//        if ((avoid.tim571_dist[index] == -1) || (avoid.tim571_rssi[index] == 0)) continue;

//        if ((avoid.tim571_dist[index] < AVOID_TIM571_DISTANCE_BAD) || 
//            (avoid.tim571_rssi[index] < AVOID_TIM571_RSSI_BAD)) continue;

        /* distance in milimeters, angle in degrees (counterclockwise, 0=x-axis)*/
        double dist = avoid.tim571_dist[index];
        double ang = (avoid.tim571_status.starting_angle + 
                      index * avoid.tim571_status.angular_step) / AVOID_TIM571_ANGLE_MULTIPLIER;

       
        if ((ang >= AVOID_ZONE1_ANGLE_MIN) && (ang <= AVOID_ZONE1_ANGLE_MAX) && 
            (dist < AVOID_ZONE1_DISTANCE)) zone1_cnt++;

        if ((ang >= AVOID_ZONE2_ANGLE_MIN) && (ang <= AVOID_ZONE2_ANGLE_MAX) && 
            (dist < AVOID_ZONE2_DISTANCE)) zone2_cnt++;

        if ((ang >= AVOID_ZONE3_ANGLE_MIN) && (ang <= AVOID_ZONE3_ANGLE_MAX) && 
            (dist < AVOID_ZONE3_DISTANCE)) zone3_cnt++;             
    }        

    xzone1_cnt = zone1_cnt;  xzone2_cnt = zone2_cnt;  xzone3_cnt = zone3_cnt;

    avoid.zone1_active = (zone1_cnt >= AVOID_ZONE1_DATA_COUNT);
    avoid.zone2_active = (zone2_cnt >= AVOID_ZONE2_DATA_COUNT);
    avoid.zone3_active = (zone3_cnt >= AVOID_ZONE3_DATA_COUNT);

    if (wheels_obstacle() ||
        (avoid.zone1_enabled && avoid.zone1_active) || 
        (avoid.zone2_enabled && avoid.zone2_active) || 
        (avoid.zone3_enabled && avoid.zone3_active)) {
        avoid.state = AVOID_STATE_STOPPED;
    } else {
        avoid.state = AVOID_STATE_UNBLOCKED;
    }

    int blocked = (avoid.state == AVOID_STATE_STOPPED);
    if (blocked != get_motor_blocked()) set_motor_blocked(blocked);

    int mask = avoid_get_zone_mask();

    avoid_log_data(avoid.state, avoid.state_old, mask != mask_old);

    if (avoid.state != avoid.state_old) {
        callback_data.avoid_state = avoid.state;

        callback_data.zone1_enabled = avoid.zone1_enabled;
        callback_data.zone1_active  = avoid.zone1_active; 
        callback_data.zone2_enabled = avoid.zone2_enabled;
        callback_data.zone2_active  = avoid.zone2_active; 
        callback_data.zone3_enabled = avoid.zone3_enabled;
        callback_data.zone3_active  = avoid.zone3_active;

        for (int i = 0; i < avoid.callbacks_count; i++) {
            avoid.callbacks[i](&callback_data);
        }
    }

    avoid.state_old = avoid.state;
}

void avoid_zone_enable(int zone, int enable)
{
    avoid_log_data(avoid.state, avoid.state_old, 1);

    switch(zone) {
    case 1:
        avoid.zone1_enabled = enable;
        break;
    case 2:
        avoid.zone2_enabled = enable;
        break;
    case 3:
        avoid.zone3_enabled = enable;
        break;
    }
}

void *avoid_thread(void *args)
{
    mikes_log(ML_INFO, "[avoid] avoid::avoid_thread(): msg=\"avoid starts\"");

    while (program_runs && !avoid.terminate) {
        avoid_data_wait();

        avoid_data_lock();
        //avoid_read_data();

        avoid_process_data();
        avoid_data_unlock();

        //usleep(20000);
    }

    mikes_log(ML_INFO, "[avoid] avoid::avoid_thread(): msg=\"avoid quits\"");
    threads_running_add(-1);
    return 0;
}

void avoid_tim571_new_data(uint16_t *dist, uint8_t *rssi, tim571_status_data *status_data)
{
    if (avoid_data_trylock() == 0) {
        memcpy(avoid.tim571_dist, dist, sizeof(uint16_t) * TIM571_DATA_COUNT);
        memcpy(avoid.tim571_rssi, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
        avoid.tim571_status = *status_data;
        alert_new_data(avoid.data_fd);
        avoid_data_unlock();
    }
}

int avoid_init()
{
    avoid.init = 0;
    avoid.terminate = 0;
    avoid.callbacks_count = 0;

    memset(&avoid.tim571_status, 0, sizeof(avoid.tim571_status));
    memset(&avoid.tim571_dist, 0, sizeof(avoid.tim571_dist));
    memset(&avoid.tim571_rssi, 0, sizeof(avoid.tim571_rssi));

    avoid.state = AVOID_STATE_UNBLOCKED;
    avoid.state_old = AVOID_STATE_NONE;

    avoid.zone1_enabled = 0; 
    avoid.zone1_active = 0;  
    avoid.zone2_enabled = 0; 
    avoid.zone2_active = 0;  
    avoid.zone3_enabled = 0; 
    avoid.zone3_active = 0;  

    avoid_log_data(avoid.state, avoid.state_old, 0);
    avoid.state_old = avoid.state;

    if (!mikes_config_use_avoid)
    {
        mikes_log(ML_INFO, "[main] avoid::avoid_init(): msg=\"avoid supressed by config.\"");
        return 0;  //fixme
    }

    if (pipe(avoid.data_fd) != 0)
    {
        perror("mikes:avoid");
        mikes_log(ML_ERR, "[main] avoid::avoid_init(): msg=\"error creating pipe!\"");
        return -2;
    }

    if (pthread_mutex_init(&avoid.data_lock, 0) != 0) 
    {
        perror("mikes:avoid");
        mikes_log(ML_ERR, "[main] avoid::avoid_init(): msg=\"error creating mutex!\"");
        return -3;
    }

    register_tim571_callback(avoid_tim571_new_data);

    if (pthread_create(&avoid.thread, 0, avoid_thread, 0) != 0)
    {
        perror("mikes:avoid");
        mikes_log(ML_ERR, "[main] avoid::avoid_init(): msg=\"error creating thread!\"");
        return -4;
    }

    threads_running_add(1);
    avoid.init = 1;

    return 0;
}

void avoid_close()
// call avoid_stop() before avoid_shutdown
{
    if (!avoid.init) return;

    unregister_tim571_callback(avoid_tim571_new_data);

    if (pthread_mutex_destroy(&avoid.data_lock) != 0) {
        mikes_log(ML_ERR, "[main] avoid::avoid_shutdown(): msg=\"error destroying mutex!\"");
    }

    close(avoid.data_fd[0]);
    close(avoid.data_fd[1]);

    avoid.init = 0;
}

int avoid_stop(void) 
{    
    if (!avoid.init) return -1;

    if (program_runs) {
        mikes_log(ML_ERR, "[main] avoid::avoid_stop(): msg=\"wrong program_runs value - unable to stop!\"");
        return -2;
    }

    avoid.terminate = 1;
    alert_new_data(avoid.data_fd);

    mikes_log(ML_INFO, "[main] avoid::avoid_stop(): msg=\"joining threads...\"");

    void *status;    
    pthread_join(avoid.thread, &status);

    mikes_log(ML_INFO, "[main] avoid::avoid_stop(): msg=\"finished\"");
    return 0;
}

int init_avoid()
{
    return avoid_init();
}

void shutdown_avoid()
{
    avoid_stop();
    avoid_close();
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------CALLBACK------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

int avoid_register_callback(avoid_data_callback callback)
{
    if (!avoid.init) return -1;

    if (avoid.callbacks_count >= AVOID_MAX_CALLBACKS)
    {
         mikes_log(ML_ERR, "[main] avoid::avoid_register_callback(): msg=\"too many avoid callbacks\"");
         return -2;
    }
    avoid.callbacks[avoid.callbacks_count++] = callback;
    return 0;
}

void avoid_unregister_callback(avoid_data_callback callback)
{
    if (!avoid.init) return;

    for (int i = 0; i < avoid.callbacks_count; i++) {
        if (avoid.callbacks[i] == callback) {
            avoid.callbacks[i] = avoid.callbacks[--avoid.callbacks_count];
        }
    }
}
