#ifndef _AVOID_H_
#define _AVOID_H_

#include <pthread.h>
#include "../live/tim571.h"

#define AVOID_MAX_CALLBACKS 20

#define AVOID_STATE_NONE       0
#define AVOID_STATE_STOPPED    1
#define AVOID_STATE_UNBLOCKED  2
#define AVOID_STATE__COUNT     3

typedef struct { 
    int avoid_state;
} avoid_callback_data_t;

typedef void (*avoid_data_callback)(avoid_callback_data_t *data);


typedef struct { 
    int init;
    int terminate;
    pthread_t thread;

    int data_fd[2];
    pthread_mutex_t data_lock;

    int callbacks_count;
    avoid_data_callback callbacks[AVOID_MAX_CALLBACKS];

    tim571_status_data tim571_status;
    uint16_t tim571_dist[TIM571_DATA_COUNT];
    uint8_t  tim571_rssi[TIM571_DATA_COUNT];

    int state;
    int state_old;
} avoid_t;


int  init_avoid();
void shutdown_avoid();

int  avoid_register_callback(avoid_data_callback callback);
void avoid_unregister_callback(avoid_data_callback callback);

#endif
