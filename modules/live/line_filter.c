#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../bites/mikes.h"
#include "../../bites/util.h"
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

int compare_lines_dist(const void *ln1, const void *ln2)
{
  line_data *ld1 = (line_data *)ln1;
  line_data *ld2 = (line_data *)ln2;
  return (ld1->distance - ld2->distance);
}

void filter_lines()
{
  // input: lines_data_local
  // output: filtered_lines
  int n = lines_data_local.line_count;
  int cluster[n];

  for (int i = 0; i < n; i++)
    cluster[i] = i;
  qsort(lines_data_local.lines, n, sizeof(line_data), compare_lines_dist);
  for (int i = 0; i < n; i++)
  {
    int maxdist = lines_data_local.lines[i].distance + LINES_SAME_CLUSTER_MAX_DISTANCE_DIFFERENCE;
    int j = i + 1;
    short alpha = lines_data_local.lines[i].angle;

    while ((j < n) && (lines_data_local.lines[j].distance <= maxdist))
    {
      short beta = lines_data_local.lines[j].angle;
      if (angle_difference(alpha, beta) < LINES_SAME_CLUSTER_MAX_ANGLE_DIFFERENCE)
        if (cluster[i] != cluster[j])
        {
          if (i < j)
          {
            for (int k = 0; k < n; k++)
              if (cluster[k] == j) cluster[k] = i;
          }
          else
          {
            for (int k = 0; k < n; k++)
              if (cluster[k] == i) cluster[k] = j;
          }
        }
      j++;
    }
  }

  for (int i = 0; i < n; i++)
    printf("%d ", cluster[i]);
  printf("\n---\n");

  uint8_t cluster_used[n];
  int cluster_size[n];
  double cluster_distance[n];
  double cluster_angle[n];
  int cluster_total_votes[n];
  int cluster_australia[n];

  memset(cluster_used, 0, n);
  memset(cluster_size, 0, sizeof(int) * n);
  memset(cluster_total_votes, 0, sizeof(int) * n);
  int cluster_count = 0;

  for (int i = 0; i < n; i++)
    cluster_total_votes[cluster[i]] += lines_data_local.lines[i].votes;

  for (int i = 0; i < n; i++)
  {
    if (!cluster_used[cluster[i]])
    {
      cluster_used[cluster[i]] = 1;
      cluster_distance[cluster[i]] = 0.0;
      cluster_angle[cluster[i]] = 0.0;
      cluster_australia[cluster[i]] = (lines_data_local.lines[i].angle + 180) % 360;
      cluster_count ++;
    }

    cluster_size[cluster[i]]++;
    double line_weight = (lines_data_local.lines[i].votes / (double) cluster_total_votes[cluster[i]]);
    cluster_distance[cluster[i]] += line_weight * lines_data_local.lines[i].distance;
    int omega = lines_data_local.lines[i].angle;
    if (omega < cluster_australia[cluster[i]]) omega += 360;
    cluster_angle[cluster[i]] += line_weight * omega;
  }

  filtered_lines.line_count = cluster_count;
  int m = 0;
  for (int i = 0; i < n; i++)
  {
    if (cluster_used[i])
    {
      filtered_lines.lines[m].distance = cluster_distance[i];
      if (cluster_angle[i] > 360) cluster_angle[i] -= 360;
      filtered_lines.lines[m].angle = cluster_angle[i];
      m++;
    }
  }
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------LIFECYCLE-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void process_new_lines()
{
  filter_lines();
  printf("filtered %d lines to %d\n", lines_data_local.line_count, filtered_lines.line_count);
  for (int i = 0; i < callbacks_count; i++)
    callbacks[i](&status_data_local_copy, &dist_local_copy, &rssi_local_copy, &filtered_lines);
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
