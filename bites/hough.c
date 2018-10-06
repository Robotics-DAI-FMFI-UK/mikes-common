#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hough.h"

#define MULTIPLIER 10000

static point_2d entry = {
  .x = 0,
  .y = 0
};

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ------------------------------CONFIG----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

int get_distance_step_count(hough_config *config)
{
  return config->distance_max / config->distance_step;
}

int get_angle_step_count(hough_config *config)
{
  return FULL_ANGLE / config->angle_step;
}

int get_votes_array_size(hough_config *config)
{
  return get_distance_step_count(config) * get_angle_step_count(config);
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// --------------------------STATUS_DATA---------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

int get_angle(tim571_status_data *status_data, int index)
{
  int value = status_data->starting_angle + index * status_data->angular_step;
  return value / MULTIPLIER;
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------INDEXING----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

int get_index_from_distance_and_angle(hough_config *config, int distance, int angle)
{
  if (distance < config->distance_max) {
    int distance_step = (distance + (config->distance_step / 2)) / config->distance_step;
    int distence_step_count = get_distance_step_count(config);
    if (distance_step >= distence_step_count) {
		distance_step = distence_step_count - 1;
	}

    int angle_step = ((angle + (config->angle_step / 2)) % FULL_ANGLE) / config->angle_step;
    int angle_step_count = get_angle_step_count(config);

    return distance_step * angle_step_count + angle_step;
  }
  return -1;
}

void get_line_data_from_index(hough_config *config, int index, line_data *line)
{
  int angle_count = get_angle_step_count(config);
  int distance = (index / angle_count) * config->distance_step;
  int angle = (index % angle_count) * config->angle_step;
  line->line.distance = distance;
  line->line.angle = angle;
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------DECISION----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

int is_good_value(hough_config *config, uint16_t distance, uint16_t rssi)
{
  return distance > config->bad_distance && rssi > config->bad_rssi;
}

int is_good_candidat(hough_config *config, int number_of_votes)
{
  if (number_of_votes > config->votes_min) {
    return 1;
  }
  return 0;
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ---------------------------COMPARATOR---------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

int line_comparator(const void *a, const void *b) {
   return ((*(line_data*)b).votes - (*(line_data*)a).votes);
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ------------------------------LOGIC-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void hough_get_lines_data(hough_config *config, tim571_status_data *status_data, uint16_t *distance, uint8_t *rssi, lines_data *data)
{
  int size = get_votes_array_size(config);
  int votes[size];
  memset(votes, 0, size * sizeof(int));

  for (int index = 0; index < status_data->data_count; index++) {
    if (is_good_value(config, distance[index], rssi[index])) {
      double dist = distance[index];
      double ang = get_angle(status_data, index);
      vector_2d v, line_vector;
      point_2d line_point;

      get_line_data_from_distance_and_angle(&dist, &ang, &v, &line_vector, &line_point);

      double distRes;
      double angRes;
      vector_2d result_vector;

      for (int angle = 0; angle < HALF_ANGLE; angle++) {
        rotate_vector_by_angle(&line_vector, &angle, &result_vector);
        find_distance_and_angle_between_point_and_line(&entry, &result_vector, &line_point, &distRes, &angRes);

        int voteIndex = get_index_from_distance_and_angle(config, distRes, angRes);
        if (voteIndex >= 0) {
          votes[voteIndex]++;
        }
      }
    }
  }

  // Take all good votes
  line_data vote_results[size];
  int result_count = 0;

  for (int vote_index = 0; vote_index < size; vote_index++) {
    if (is_good_candidat(config, votes[vote_index])) {
      line_data line;

      get_line_data_from_index(config, vote_index, &line);
      line.votes = votes[vote_index];

      vote_results[result_count++] = line;
    }
  }

  // Sort MAXIMUM_POSSIBLES votes
  qsort(vote_results, result_count, sizeof(line_data), line_comparator);

  // Take first best fit LINE_MAX_DATA_COUNT from MAXIMUM_POSSIBLES
  for (int index = 0; index < LINE_MAX_DATA_COUNT && index < result_count; index++) {
    data->lines[index] = vote_results[index];
  }
  data->count = result_count < LINE_MAX_DATA_COUNT ? result_count : LINE_MAX_DATA_COUNT;
}

void hough_print_lines_data(lines_data *data)
{
  printf("LinesData:\n");
  for (int index = 0; index < data->count; index++) {
    printf("%d: Distance: %10.4f, Angle: %10.4f, Votes: %10d\n", index, data->lines[index].line.distance, data->lines[index].line.angle, data->lines[index].votes);
  }
}
