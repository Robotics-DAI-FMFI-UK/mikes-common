#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../bites/mikes.h"
#include "../../bites/filter.h"
#include "line_filter.h"
#include "../passive/mikes_logs.h"
#include "core/config_mikes.h"

#define MAX_LINE_FILTER_CALLBACKS 20

#define LINES_SAME_CLUSTER_MAX_DISTANCE_DIFFERENCE 150
#define LINES_SAME_CLUSTER_MAX_ANGLE_DIFFERENCE 15

static pthread_mutex_t      line_filter_lock;
static int                  fd[2];

static uint16_t             dist_local_copy[TIM571_DATA_COUNT];
static uint8_t              rssi_local_copy[TIM571_DATA_COUNT];
static tim571_status_data   status_data_local_copy;
static lines_data           lines_data_local;

static lines_data           filtered_lines;

static line_filter_callback  callbacks[MAX_LINE_FILTER_CALLBACKS];
static int                   callbacks_count;

static int online;

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------LIFECYCLE-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void process_new_lines()
{
  filter_lines(&lines_data_local, &filtered_lines);
  for (int i = 0; i < callbacks_count; i++)
    callbacks[i](&status_data_local_copy, dist_local_copy, rssi_local_copy, &filtered_lines);
}

void *line_filter_thread(void *args)
{
  while (program_runs)
  {
    if (wait_for_new_data(fd) < 0) {
      perror("mikes:line_filter");
      mikes_log(ML_ERR, "line_filter error during waiting on new Data.");
      continue;
    }
    pthread_mutex_lock(&line_filter_lock);
    process_new_lines();
    pthread_mutex_unlock(&line_filter_lock);
  }

  mikes_log(ML_INFO, "line_filter quits.");
  threads_running_add(-1);
  return 0;
}

void new_tim_hough_data_arrived(tim571_status_data *status_data, uint16_t *distance, uint8_t *rssi, lines_data *new_lines)
{
  if (pthread_mutex_trylock(&line_filter_lock) == 0) {
    status_data_local_copy = *status_data;
    memcpy(dist_local_copy, distance, sizeof(uint16_t) * TIM571_DATA_COUNT);
    memcpy(rssi_local_copy, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
    lines_data_local = *new_lines;
    alert_new_data(fd);
    pthread_mutex_unlock(&line_filter_lock);
  }
}

void init_line_filter()
{
  if (!mikes_config.use_line_filter)
  {
    mikes_log(ML_INFO, "line_filter supressed by config.");
    online = 0;
    return;
  }
  online = 1;

  if (pipe(fd) != 0)
  {
    perror("mikes:line_filter");
    mikes_log(ML_ERR, "creating pipe for line_filter");
    return;
  }

  pthread_t t;
  pthread_mutex_init(&line_filter_lock, 0);
  register_tim_hough_transform_callback(new_tim_hough_data_arrived);

  if (pthread_create(&t, 0, line_filter_thread, 0) != 0)
  {
    perror("mikes:line_filter");
    mikes_log(ML_ERR, "creating thread for line_filter");
  }
  else threads_running_add(1);
}

void shutdown_line_filter()
{
  close(fd[0]);
  close(fd[1]);
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------CALLBACK------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void register_line_filter_callback(line_filter_callback callback)
{
  if (!online) return;

  if (callbacks_count >= MAX_LINE_FILTER_CALLBACKS)
  {
     mikes_log(ML_ERR, "too many line filter callbacks");
     return;
  }
  callbacks[callbacks_count++] = callback;
}

void unregister_line_filter_callback(line_filter_callback callback)
{
  if (!online) return;

  for (int i = 0; i < callbacks_count; i++) {
    if (callbacks[i] == callback) {
       callbacks[i] = callbacks[(callbacks_count--) - 1];
    }
  }
}
