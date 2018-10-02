#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "tim_segment.h"
#include "line_filter.h"

#include "../../bites/mikes.h"
#include "../../bites/util.h"
#include "../passive/mikes_logs.h"
#include "core/config_mikes.h"

#define MAX_TIM_SEGMENT_CALLBACKS 20

static pthread_mutex_t      tim_segment_lock;
static int                  fd[2];

static uint16_t             dist_local_copy[TIM571_DATA_COUNT];
static uint8_t              rssi_local_copy[TIM571_DATA_COUNT];
static tim571_status_data   status_data_local_copy;
static lines_data           lines_data_local_copy;

static lines_data           segments_data;

static tim_segment_receive_data_callback  callbacks[MAX_TIM_SEGMENT_CALLBACKS];
static int                                callbacks_count;

static int online;

static segment_config tim_segment_default_config = {
  .max_distance_error = 10.0,
  .min_points_segment = 5,
  .max_points_skip = 5,
  .bad_rssi = 0
};

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------LIFECYCLE-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void process_new_data()
{
  segment_transform_points_and_lines_to_segments(&tim_segment_default_config, &status_data_local_copy, dist_local_copy, rssi_local_copy, &lines_data_local_copy, &segments_data);
  for (int i = 0; i < callbacks_count; i++)
    callbacks[i](&segments_data);
}

void line_filter_new_data(tim571_status_data *status_data, uint16_t *distance, uint8_t *rssi, lines_data *lines)
{
  if (pthread_mutex_trylock(&tim_segment_lock) == 0) {
    status_data_local_copy = *status_data;
    memcpy(dist_local_copy, distance, sizeof(uint16_t) * TIM571_DATA_COUNT);
    memcpy(rssi_local_copy, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
    lines_data_local_copy = *lines;
    alert_new_data(fd);
    pthread_mutex_unlock(&tim_segment_lock);
  }
}

void *tim_segment_thread(void *args)
{
  while (program_runs)
  {
    if (wait_for_new_data(fd) < 0) {
      perror("mikes:tim_segment");
      mikes_log(ML_ERR, "tim_segment error during waiting on new Data.");
      continue;
    }
    pthread_mutex_lock(&tim_segment_lock);
    process_new_data();
    pthread_mutex_unlock(&tim_segment_lock);
  }

  mikes_log(ML_INFO, "line_filter quits.");
  threads_running_add(-1);
  return 0;
}

void init_tim_segment()
{
  if (!mikes_config.use_tim_segment)
  {
    mikes_log(ML_INFO, "tim_segment supressed by config.");
    online = 0;
    return;
  }
  online = 1;

  if (pipe(fd) != 0)
  {
    perror("mikes:tim_segment");
    mikes_log(ML_ERR, "creating pipe for tim segment");
    return;
  }

  pthread_t t;
  pthread_mutex_init(&tim_segment_lock, 0);
  register_line_filter_callback(line_filter_new_data);
  if (pthread_create(&t, 0, tim_segment_thread, 0) != 0)
  {
    perror("mikes:tim_segment");
    mikes_log(ML_ERR, "creating thread for tim segment");
  }
  else threads_running_add(1);
}

void shutdown_tim_segment()
{
  close(fd[0]);
  close(fd[1]);
}

void tim_segment_change_default_config(segment_config *config)
{
  pthread_mutex_lock(&tim_segment_lock);
  tim_segment_default_config = *config;
  pthread_mutex_unlock(&tim_segment_lock);
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------CALLBACK------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void register_tim_segment_callback(tim_segment_receive_data_callback callback)
{
  if (!online) return;

  if (callbacks_count >= MAX_LINE_FILTER_CALLBACKS)
  {
     mikes_log(ML_ERR, "too many time segment callbacks");
     return;
  }
  callbacks[callbacks_count++] = callback;
}

void unregister_tim_segment_callback(tim_segment_receive_data_callback callback)
{
  if (!online) return;

  for (int i = 0; i < callbacks_count; i++) {
    if (callbacks[i] == callback) {
       callbacks[i] = callbacks[(callbacks_count--) - 1];
    }
  }
}
