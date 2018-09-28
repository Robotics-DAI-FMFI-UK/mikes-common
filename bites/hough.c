#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "hough.h"

#define MULTIPLIER 10000
#define FULL_ANGLE 360
#define HALF_ANGLE 180
#define RADIAN (M_PI / HALF_ANGLE)

typedef struct vector_struct {
  double x;
  double y;
} vector;

typedef struct point_struct {
  double x;
  double y;
} point;

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
    int step = distance / config->distance_step;
    return (step * get_angle_step_count(config)) + (angle % HALF_ANGLE) / config->angle_step;
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

  double result_angle = angle_from_axis_x(&cross_v);
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

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -------------------------------------TEST------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void test_get_distance_step_count(hough_config *config)
{
  printf("Test get distance step count:\n");
  printf("For distance_max %d distance_step %d => %d\n", config->distance_max, config->distance_step, get_distance_step_count(config));
  printf("\n");
}

void test_get_angle_step_count(hough_config *config)
{
  printf("Test get angle step count:\n");
  printf("For FULL_ANGLE %d angle_step %d => %d\n", FULL_ANGLE, config->angle_step, get_angle_step_count(config));
  printf("\n");
}

void test_get_votes_array_size(hough_config *config)
{
  printf("Test get angle step count:\n");
  printf("For distance_max %d distance_step %d FULL_ANGLE %d angle_step %d => %d\n", config->distance_max, config->distance_step, FULL_ANGLE, config->angle_step, get_votes_array_size(config));
  printf("\n");
}

void test_get_angle(tim571_status_data *status_data)
{
  printf("Test get angle:\n");
  printf("For index %d => %d\n", 0, get_angle(status_data, 0));
  printf("For index %d => %d\n", 135, get_angle(status_data, 135));
  printf("For index %d => %d\n", 810, get_angle(status_data, 810));
  printf("\n");
}

void test_get_index_from_distance_and_angle(hough_config *config)
{
  printf("Test get index from distance and angle (VOTE_ARRAY_SIZE %d):\n", get_votes_array_size(config));
  printf("For distance %d angle %d => %d\n", 0, 0, get_index_from_distance_and_angle(config, 0, 0));
  printf("For distance %d angle %d => %d\n", 194, 133, get_index_from_distance_and_angle(config, 194, 133));
  printf("For distance %d angle %d => %d\n", config->distance_max - 1, 179, get_index_from_distance_and_angle(config, config->distance_max - 1, 179));
  printf("For distance %d angle %d => %d\n", config->distance_max - 1, 359, get_index_from_distance_and_angle(config, config->distance_max - 1, 359));
  printf("\n");
}

void test_get_line_data_from_index_unit(hough_config *config, int index, line_data *line)
{
  get_line_data_from_index(config, index, line);
  printf("For index %d => distance %d angle %d\n", index, line->distance, line->angle);
}

void test_get_line_data_from_index(hough_config *config)
{
  printf("Test get line data from index (toleration %d):\n", config->distance_step);
  line_data line;

  test_get_line_data_from_index_unit(config, 0, &line);
  test_get_line_data_from_index_unit(config, 4453, &line);
  test_get_line_data_from_index_unit(config, get_votes_array_size(config) - 1, &line);

  printf("\n");
}

void test_angle_to_vector_unit(int angle, vector *v)
{
  angle_to_vector(angle, v);
  printf("For angle %d => x %f y %f\n", angle, v->x, v->y);
}

void test_angle_to_vector()
{
  printf("Test angle to vector:\n");
  vector v;

  test_angle_to_vector_unit(0, &v);
  test_angle_to_vector_unit(45, &v);
  test_angle_to_vector_unit(90, &v);
  test_angle_to_vector_unit(180, &v);
  test_angle_to_vector_unit(270, &v);

  printf("\n");
}

void test_get_perpendicular_vector_unit(int x, int y)
{
  vector v, perpendicular;
  v.x = x;
  v.y = y;
  get_perpendicular_vector(&v, &perpendicular);
  printf("For vector (%f, %f) => (%f, %f)\n", v.x, v.y, perpendicular.x, perpendicular.y);
}

void test_get_perpendicular_vector()
{
  printf("Test get perpendicular vector:\n");

  test_get_perpendicular_vector_unit(1, 0);
  test_get_perpendicular_vector_unit(0, -1);

  printf("\n");
}

void test_get_point_from_vector_and_distance_unit(double x, double y, int distance)
{
  vector v;
  point p;
  v.x = x;
  v.y = y;
  get_point_from_vector_and_distance(&v, &distance, &p);
  printf("For vector (%f, %f) distance %d => (%f, %f)\n", v.x, v.y, distance, p.x, p.y);
}

void test_get_point_from_vector_and_distance()
{
  printf("Test get point from vector and distance:\n");

  test_get_point_from_vector_and_distance_unit(1, 0, 100);
  test_get_point_from_vector_and_distance_unit(0, -1, 100);
  test_get_point_from_vector_and_distance_unit(0.707107, 0.707107, 100);

  printf("\n");
}

void test_rotate_vector_by_angle_unit(double x, double y, int angle)
{
  vector v, r;
  v.x = x;
  v.y = y;
  rotate_vector_by_angle(&v, &angle, &r);
  printf("For vector (%f, %f) angle %d => (%f, %f)\n", v.x, v.y, angle, r.x, r.y);
}

void test_rotate_vector_by_angle()
{
  printf("Test get point from vector and distance:\n");

  test_rotate_vector_by_angle_unit(1, 0, 45);
  test_rotate_vector_by_angle_unit(1, 0, 90);

  printf("\n");
}

void test_get_vector_length_unit(double x, double y)
{
  vector v;
  v.x = x;
  v.y = y;
  printf("For vector (%f, %f) => %f\n", v.x, v.y, get_vector_length(&v));
}

void test_get_vector_length()
{
  printf("Test get vector length:\n");

  test_get_vector_length_unit(1, 0);
  test_get_vector_length_unit(10, 0);
  test_get_vector_length_unit(-10, 0);
  test_get_vector_length_unit(0, -10);

  printf("\n");
}

void test_distance_of_point_from_line_unit(double px, double py, double sx, double sy, double lx, double ly)
{
  point p;
  p.x = px;
  p.y = py;
  vector line_directional;
  line_directional.x = sx;
  line_directional.y = sy;
  point line_point;
  line_point.x = lx;
  line_point.y = ly;

  printf("For point [%f, %f] line directional vector (%f, %f) line point [%f, %f] => distance %f\n",
          p.x, p.y, line_directional.x, line_directional.y, line_point.x, line_point.y,
          distance_of_point_from_line(&p, &line_directional, &line_point));
}

void test_distance_of_point_from_line()
{
  printf("Test distance of point from line:\n");

  test_distance_of_point_from_line_unit(0, 0, 1, 0, 3, 1);
  test_distance_of_point_from_line_unit(0, 0, 0, 1, 3, 1);

  printf("\n");
}

void test_angle_from_axis_x_unit(double vx, double vy)
{
  vector v;
  v.x = vx;
  v.y = vy;
  printf("Angle of vector (%f, %f) => %f\n", v.x, v.y, angle_from_axis_x(&v));
}

void test_angle_from_axis_x()
{
  printf("Test angle from axis x:\n");

  test_angle_from_axis_x_unit(1, 0);
  test_angle_from_axis_x_unit(4, 0);
  test_angle_from_axis_x_unit(4, 4);
  test_angle_from_axis_x_unit(0, 3);
  test_angle_from_axis_x_unit(0, -3);
  test_angle_from_axis_x_unit(-0, 3);

  printf("\n");
}

void test_find_cross_of_two_lines_unit(double v1x, double v1y, double p1x, double p1y, double v2x, double v2y, double p2x, double p2y)
{
  vector v1, v2;
  point p1, p2, cross;
  v1.x = v1x;
  v1.y = v1y;
  p1.x = p1x;
  p1.y = p1y;
  v2.x = v2x;
  v2.y = v2y;
  p2.x = p2x;
  p2.y = p2y;
  find_cross_of_two_lines(&v1, &p1, &v2, &p2, &cross);
  printf("For vector (%f; %f) point [%f; %f] and vector (%f; %f) point [%f; %f] => cross [%f; %f]\n",
          v1.x, v1.y, p1.x, p1.y, v2.x, v2.y, p2.x, p2.y,
          cross.x, cross.y);
}

void test_find_cross_of_two_lines()
{
  printf("Test find cross of two lines:\n");

  test_find_cross_of_two_lines_unit(0, -1, 3, 1, 1, 0, 0, 0);
  test_find_cross_of_two_lines_unit(1, 0, 3, 1, 0, 1, 0, 0);

  printf("\n");
}

void test_vector_and_point_to_distance_and_angle_unit(double vx, double vy, double px, double py)
{
  int distance, angle;

  vector v;
  v.x = vx;
  v.y = vy;

  point p;
  p.x = px;
  p.y = py;

  vector_and_point_to_distance_and_angle(&v, &p, &distance, &angle);
  printf("For vector (%f, %f) point (%f, %f) => distance %d angle %d\n", v.x, v.y, p.x, p.y, distance, angle);
}

void test_vector_and_point_to_distance_and_angle()
{
  printf("Test vector and point to distance and angle:\n");

  test_vector_and_point_to_distance_and_angle_unit(1, 0, 3, 1);
  test_vector_and_point_to_distance_and_angle_unit(-1, 0, 3, 1);
  test_vector_and_point_to_distance_and_angle_unit(1, 0, -3, 1);
  test_vector_and_point_to_distance_and_angle_unit(1, 0, 1, 3);
  test_vector_and_point_to_distance_and_angle_unit(1, 0, 1, -3);
  test_vector_and_point_to_distance_and_angle_unit(0, 1, 3, 1);
  test_vector_and_point_to_distance_and_angle_unit(0, -1, 3, 1);

  printf("\n");
}

void test_hough_methods(hough_config *config, tim571_status_data *status_data, uint16_t *distance, uint8_t *rssi)
{
  test_get_distance_step_count(config);
  test_get_angle_step_count(config);
  test_get_votes_array_size(config);
  test_get_angle(status_data);
  test_get_index_from_distance_and_angle(config);
  test_get_line_data_from_index(config);
  test_angle_to_vector();
  test_get_perpendicular_vector();
  test_get_point_from_vector_and_distance();
  test_rotate_vector_by_angle();
  test_get_vector_length();
  test_distance_of_point_from_line();
  test_angle_from_axis_x();
  test_find_cross_of_two_lines();
  test_vector_and_point_to_distance_and_angle();
}
