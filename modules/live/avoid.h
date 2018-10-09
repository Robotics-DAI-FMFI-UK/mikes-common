#ifndef _AVOID_H_
#define _AVOID_H_

#include <pthread.h>
#include "../live/tim571.h"

#define AVOID_MAX_CALLBACKS 20

#define AVOID_MSG_TYPE_NONE       0
#define AVOID_MSG_TYPE_STOPPED    1
#define AVOID_MSG_TYPE_UNBLOCKED  2
#define AVOID_MSG_TYPE__COUNT     3

typedef struct { 
    int avoid_msg_type;
} avoid_callback_data_t;

typedef void (*avoid_data_callback)(avoid_callback_data_t *data);


typedef struct { 
    int init;
    pthread_t thread;

    int data_fd[2];
    pthread_mutex_t data_lock;

    int callbacks_count;
    avoid_data_callback callbacks[AVOID_MAX_CALLBACKS];

    tim571_status_data status_data_local_copy;
    uint16_t dist_local_copy[TIM571_DATA_COUNT];
    uint8_t  rssi_local_copy[TIM571_DATA_COUNT];

} avoid_t;


int  init_avoid();
void shutdown_avoid();

int  avoid_register_callback(avoid_data_callback callback);
void avoid_unregister_callback(avoid_data_callback callback);

#endif
