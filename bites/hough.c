#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "hough.h"

#define MULTIPLIER 10000
#define HALF_ANGLE 180
#define RADIAN (M_PI / HALF_ANGLE)

static point entry = {
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
  line->distance = distance;
  line->angle = angle;
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
// -------------------------------MATH-----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void angle_to_vector(int angle, vector *v)
{
  double rad = RADIAN * angle;
  v->x = cos(rad);
  v->y = sin(rad);
}

void get_perpendicular_vector(vector *v, vector *perpendicular)
{
  perpendicular->x = v->y;
  perpendicular->y = -v->x;
}

void get_point_from_vector_and_distance(vector *v, int *distance, point *p)
{
  p->x = v->x * *distance;
  p->y = v->y * *distance;
}

void rotate_vector_by_angle(vector *line, int *angle, vector *result)
{
  double rad = RADIAN * *angle;
  double cosRad = cos(rad);
  double sinRad = sin(rad);
  result->x = cosRad * line->x - sinRad * line->y;
  result->y = sinRad * line->x + cosRad * line->y;
}

double get_vector_length(vector *v)
{
  return sqrt(v->x * v->x + v->y * v->y);
}

double get_c_from_vector_and_point(vector *normal_v, point *p)
{
  return -(normal_v->x * p->x + normal_v->y *p->y);
}

// Not currently using
double distance_of_point_from_line(point *p, vector *line_direction_v, point *line_p)
{
  vector perpendicular;
  get_perpendicular_vector(line_direction_v, &perpendicular);

  return (perpendicular.x * p->x + perpendicular.y * p->y + get_c_from_vector_and_point(&perpendicular, line_p)) / get_vector_length(&perpendicular);
}

double angle_from_axis_x(vector *v)
{
  return atan2(v->y, v->x) / RADIAN;
}

void vector_from_two_points(point *p1, point *p2, vector *v)
{
  v->x = p2->x - p1->x;
  v->y = p2->y - p1->y;
}

void find_cross_of_two_lines(vector *normal_v1, point *p1, vector *normal_v2, point *p2, point *result)
{
  double c1 = get_c_from_vector_and_point(normal_v1, p1);
  double c2 = get_c_from_vector_and_point(normal_v2, p2);

  if (normal_v2->x != 0) { // normal_v2.x cant be 0
    result->y = ((c2 * normal_v1->x) - (c1 * normal_v2->x)) / ((normal_v1->y * normal_v2->x) - (normal_v1->x * normal_v2->y));
    result->x = -(normal_v2->y * result->y + c2) / normal_v2->x;
  } else if (normal_v2->y != 0) { // normal_v2.y cant be 0
    result->x = ((c2 * normal_v1->y) - (c1 * normal_v2->y)) / ((normal_v1->x * normal_v2->y) - (normal_v1->y * normal_v2->x));
    result->y = -(normal_v2->x * result->x + c2) / normal_v2->y;
  } else {
    printf("Unexpected error normal_v2.x and normal_v2.y are both 0");
  }
}

void vector_and_point_to_distance_and_angle(vector *line_directional, point *line_p, int *distance, int *angle)
{
  vector perpendicular_v;
  get_perpendicular_vector(line_directional, &perpendicular_v);

  point cross;
  find_cross_of_two_lines(&perpendicular_v, line_p, line_directional, &entry, &cross);

  vector cross_v;
  vector_from_two_points(&entry, &cross, &cross_v);

  *distance = get_vector_length(&cross_v);

  double result_angle;
  if (cross_v.x == 0 && cross_v.y == 0) {
    result_angle = angle_from_axis_x(line_directional) + 90;
  } else {
    result_angle = angle_from_axis_x(&cross_v);
  }

  *angle = (((int) result_angle) + 360) % 360;
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
      int dist = distance[index];
      int ang = get_angle(status_data, index);
      vector v, line_vector;
      point line_point;

      angle_to_vector(ang, &v);
      get_perpendicular_vector(&v, &line_vector);
      get_point_from_vector_and_distance(&v, &dist, &line_point);

      int distRes;
      int angRes;
      vector result_vector;

      for (int angle = 0; angle < HALF_ANGLE; angle++) {
        rotate_vector_by_angle(&line_vector, &angle, &result_vector);
        vector_and_point_to_distance_and_angle(&result_vector, &line_point, &distRes, &angRes);

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
  data->line_count = result_count < LINE_MAX_DATA_COUNT ? result_count : LINE_MAX_DATA_COUNT;
}

void printf_lines_data(lines_data *data)
{
  printf("LinesData:\n");
  for (int index = 0; index < data->line_count; index++) {
    printf("%d: Distance: %10d, Angle: %3d, Votes: %10d\n", index, data->lines[index].distance, data->lines[index].angle, data->lines[index].votes);
  }
}

