#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "../../bites/mikes.h"
#include "hough_transform.h"
#include "../passive/mikes_logs.h"
#include "core/config_mikes.h"

static pthread_mutex_t      tim571_new_data_lock;
static pthread_mutex_t      hough_transform_lock;

static uint16_t             dist_local_copy[TIM571_DATA_COUNT];
static uint8_t              rssi_local_copy[TIM571_DATA_COUNT];
static tim571_status_data   status_data_local_copy;

static lines_data           lines_data_local;

static hough_transform_receive_data_callback  callbacks[MAX_HOUGH_TRANSFORM_CALLBACKS];
static int                                    callbacks_count;

static int online;

static hough_config default_config = {
  .distance_max = TIM571_MAX_DISTANCE,
  .distance_step = 15,
  .angle_max = 360,
  .angle_step = 5,
  .votes_min = 10,
  .bad_distance = 0,
  .bad_rssi = 0
};

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------LIFECYCLE-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void change_default_config(hough_config *config)
{
  pthread_mutex_lock(&hough_transform_lock);
  memcpy(config, &default_config, sizeof hough_config);
  pthread_mutex_unlock(&hough_transform_lock);
}

void process_tim571_data()
{
  hough_get_lines_data(&default_config, &status_data_local_copy, &dist_local_copy, &rssi_local_copy, &lines_data_local);
  for (int i = 0; i < callbacks_count; i++)
    callbacks[i](&lines_data_local);
}

void *hough_transform_thread(void *args)
{
  pthread_mutex_lock(&tim571_new_data_lock); // Wait for first data

  while (program_runs && pthread_mutex_lock(&tim571_new_data_lock) == 0)
  {
    pthread_mutex_lock(&hough_transform_lock);
    process_tim571_data();
    pthread_mutex_unlock(&hough_transform_lock);
  }

  mikes_log(ML_INFO, "hough_transform quits.");
  threads_running_add(-1);
  return 0;
}

void tim571_new_data(uint16_t *dist, uint8_t *rssi, tim571_status_data *status_data)
{
  if (pthread_mutex_trylock(&hough_transform_lock) == 0) {
    memcpy(dist_local_copy, dist, sizeof(uint16_t) * TIM571_DATA_COUNT);
    memcpy(rssi_local_copy, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
    memcpy(status_data, &status_data_local_copy, sizeof tim571_status_data);
    pthread_mutex_unlock(&hough_transform_lock);
    pthread_mutex_unlock(&tim571_new_data_lock);
  }
}

void init_hough_transform()
{
  if (!mikes_config.use_hough_transform)
  {
    mikes_log(ML_INFO, "hough_transform supressed by config.");
    online = 0;
    return;
  }
  online = 1;

  pthread_t t;
  register_tim571_callback(tim571_new_data);
  pthread_mutex_init(&tim571_new_data_lock, 0);
  pthread_mutex_init(&hough_transform_lock, 0);
  if (pthread_create(&t, 0, hough_transform_thread, 0) != 0)
  {
    perror("mikes:hough_transform");
    mikes_log(ML_ERR, "creating thread for hough transform");
  }
  else threads_running_add(1);
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------CALLBACK-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void register_hough_transform_callback(hough_transform_receive_data_callback callback)
{
  if (!online) return;

  if (callbacks_count >= MAX_HOUGH_TRANSFORM_CALLBACKS)
  {
     mikes_log(ML_ERR, "too many hough_transform callbacks");
     return;
  }
  callbacks[callbacks_count++] = callback;
}

void unregister_hough_transform_callback(hough_transform_receive_data_callback callback)
{
  if (!online) return;

  for (int i = 0; i < callbacks_count; i++) {
    if (callbacks[i] == callback) {
       callbacks[i] = callbacks[(callbacks_count--) - 1];

    }
  }
}
