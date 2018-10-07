#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "../../bites/mikes.h"
#include "tim_corner.h"
#include "../passive/mikes_logs.h"
#include "core/config_mikes.h"
#include "../../bites/util.h"
#include "tim_segment.h"

#define MAX_TIM_CORNER_CALLBACKS 20

static pthread_mutex_t      tim_corner_lock;
static int                  fd[2];

static segments_data        segments_local_copy;

static corners_data         corners_data_local;

static tim_corner_receive_data_callback  callbacks[MAX_TIM_CORNER_CALLBACKS];
static int                               callbacks_count;

static int online;

static corner_config tim_corner_default_config = {
  .max_tolerance_angle = 5.0,
  .max_tolerance_distance = 20.0
};

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------LIFECYCLE-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void tim_corner_change_default_config(corner_config *config)
{
  pthread_mutex_lock(&tim_corner_lock);
  tim_corner_default_config = *config;
  pthread_mutex_unlock(&tim_corner_lock);
}

void process_segments_data()
{
  corner_find_from_segments(&tim_corner_default_config, &segments_local_copy, &corners_data_local);
  for (int i = 0; i < callbacks_count; i++)
    callbacks[i](&corners_data_local);
}

void *tim_corner_thread(void *args)
{
  while (program_runs)
  {
    if (wait_for_new_data(fd) < 0) {
      perror("mikes:tim_corner");
      mikes_log(ML_ERR, "tim_corner error during waiting on new Data.");
      continue;
    }
    pthread_mutex_lock(&tim_corner_lock);
    process_segments_data();
    pthread_mutex_unlock(&tim_corner_lock);
  }

  mikes_log(ML_INFO, "tim_corner quits.");
  threads_running_add(-1);
  return 0;
}

void segments_new_data(segments_data *segments)
{
  if (pthread_mutex_trylock(&tim_corner_lock) == 0) {
    segments_local_copy = *segments;
    alert_new_data(fd);
    pthread_mutex_unlock(&tim_corner_lock);
  }
}

void init_tim_corner()
{
  if (!mikes_config.use_tim_corner)
  {
    mikes_log(ML_INFO, "tim_corner supressed by config.");
    online = 0;
    return;
  }
  online = 1;

  if (pipe(fd) != 0)
  {
    perror("mikes:tim_corner");
    mikes_log(ML_ERR, "creating pipe for tim corner");
    return;
  }

  pthread_t t;
  pthread_mutex_init(&tim_corner_lock, 0);
  register_tim_segment_callback(segments_new_data);
  if (pthread_create(&t, 0, tim_corner_thread, 0) != 0)
  {
    perror("mikes:tim_corner");
    mikes_log(ML_ERR, "creating thread for tim corner");
  }
  else threads_running_add(1);
}

void shutdown_tim_corner()
{
  close(fd[0]);
  close(fd[1]);
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------CALLBACK------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void register_tim_corner_callback(tim_corner_receive_data_callback callback)
{
  if (!online) return;

  if (callbacks_count >= MAX_TIM_CORNER_CALLBACKS)
  {
     mikes_log(ML_ERR, "too many tim_corner callbacks");
     return;
  }
  callbacks[callbacks_count++] = callback;
}

void unregister_tim_corner_callback(tim_corner_receive_data_callback callback)
{
  if (!online) return;

  for (int i = 0; i < callbacks_count; i++) {
    if (callbacks[i] == callback) {
       callbacks[i] = callbacks[(callbacks_count--) - 1];
    }
  }
}
