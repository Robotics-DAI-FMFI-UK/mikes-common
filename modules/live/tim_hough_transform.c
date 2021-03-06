#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "../../bites/mikes.h"
#include "tim_hough_transform.h"
#include "../passive/mikes_logs.h"
#include "core/config_mikes.h"
#include "../../bites/util.h"

#define MAX_TIM_HOUGH_TRANSFORM_CALLBACKS 20

static pthread_mutex_t      tim_hough_transform_lock;
static pthread_mutex_t      tim_hough_transform_mode_lock;
static int                  fd[2];

static uint16_t             dist_local_copy[TIM571_DATA_COUNT];
static uint8_t              rssi_local_copy[TIM571_DATA_COUNT];
static tim571_status_data   status_data_local_copy;

static lines_data           lines_data_local;

static tim_hough_transform_receive_data_callback  callbacks[MAX_TIM_HOUGH_TRANSFORM_CALLBACKS];
static int                                        callbacks_count;

static int online;

static int current_mode = TIM_HOUGH_TRANSFORM_MODE_CONTINUOUS;
static int compute_next_tick = 0;

static hough_config tim_hough_default_config = {
  .distance_max = TIM571_MAX_DISTANCE,
  .distance_step = 20,
  .angle_step = 1,
  .votes_min = 15,
  .bad_distance = 0,
  .bad_rssi = 0
};

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------LIFECYCLE-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

int tim_hough_transform_should_compute()
{
  return current_mode == TIM_HOUGH_TRANSFORM_MODE_CONTINUOUS || compute_next_tick;
}

void tim_hough_transform_do_compute_once()
{
  if (!online) return;

  pthread_mutex_lock(&tim_hough_transform_lock);
  compute_next_tick = 1;
  pthread_mutex_unlock(&tim_hough_transform_lock);
}

void tim_hough_transform_set_mode(int mode)
{
  if (!online) return;

  pthread_mutex_lock(&tim_hough_transform_mode_lock);
  current_mode = mode;
  pthread_mutex_unlock(&tim_hough_transform_mode_lock);
}

void tim_hough_transform_change_default_config(hough_config *config)
{
  if (!online) return;

  pthread_mutex_lock(&tim_hough_transform_lock);
  memcpy(config, &tim_hough_default_config, sizeof(hough_config));
  pthread_mutex_unlock(&tim_hough_transform_lock);
}

void process_tim571_data()
{
  hough_get_lines_data(&tim_hough_default_config, &status_data_local_copy, dist_local_copy, rssi_local_copy, &lines_data_local);
  for (int i = 0; i < callbacks_count; i++)
    callbacks[i](&status_data_local_copy, dist_local_copy, rssi_local_copy, &lines_data_local);
}

void *tim_hough_transform_thread(void *args)
{
  while (program_runs)
  {
    if (wait_for_new_data(fd) < 0) {
      perror("mikes:tim_hough_transform");
      mikes_log(ML_ERR, "tim_hough_transform error during waiting on new Data.");
      continue;
    }
    pthread_mutex_lock(&tim_hough_transform_lock);
    process_tim571_data();
    pthread_mutex_unlock(&tim_hough_transform_lock);
  }

  mikes_log(ML_INFO, "tim_hough_transform quits.");
  threads_running_add(-1);
  return 0;
}

void tim571_new_data(uint16_t *dist, uint8_t *rssi, tim571_status_data *status_data)
{
  pthread_mutex_lock(&tim_hough_transform_mode_lock);
  if (tim_hough_transform_should_compute() && pthread_mutex_trylock(&tim_hough_transform_lock) == 0) {
    memcpy(dist_local_copy, dist, sizeof(uint16_t) * TIM571_DATA_COUNT);
    memcpy(rssi_local_copy, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
    status_data_local_copy = *status_data;
    compute_next_tick = 0;
    alert_new_data(fd);
    pthread_mutex_unlock(&tim_hough_transform_lock);
  }
  pthread_mutex_unlock(&tim_hough_transform_mode_lock);
}

void init_tim_hough_transform()
{
  if (!mikes_config.use_tim_hough_transform)
  {
    mikes_log(ML_INFO, "tim_hough_transform supressed by config.");
    online = 0;
    return;
  }
  online = 1;

  if (pipe(fd) != 0)
  {
    perror("mikes:tim_hough_transform");
    mikes_log(ML_ERR, "creating pipe for tim hough transform");
    return;
  }

  pthread_t t;
  pthread_mutex_init(&tim_hough_transform_lock, 0);
  pthread_mutex_init(&tim_hough_transform_mode_lock, 0);
  register_tim571_callback(tim571_new_data);
  if (pthread_create(&t, 0, tim_hough_transform_thread, 0) != 0)
  {
    perror("mikes:tim_hough_transform");
    mikes_log(ML_ERR, "creating thread for tim hough transform");
  }
  else threads_running_add(1);
}

void shutdown_tim_hough_transform()
{
  close(fd[0]);
  close(fd[1]);
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------CALLBACK-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void register_tim_hough_transform_callback(tim_hough_transform_receive_data_callback callback)
{
  if (!online) return;

  if (callbacks_count >= MAX_TIM_HOUGH_TRANSFORM_CALLBACKS)
  {
     mikes_log(ML_ERR, "too many tim_hough_transform callbacks");
     return;
  }
  callbacks[callbacks_count++] = callback;
}

void unregister_tim_hough_transform_callback(tim_hough_transform_receive_data_callback callback)
{
  if (!online) return;

  for (int i = 0; i < callbacks_count; i++) {
    if (callbacks[i] == callback) {
       callbacks[i] = callbacks[(callbacks_count--) - 1];

    }
  }
}
