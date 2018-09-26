#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "hough.h"

#define MULTIPLIER 10000
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
  return config->angle_max / config->angle_step;
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

// https://math.stackexchange.com/questions/310653/finding-the-angle-theta-between-two-2d-vectors
double get_radians_between_2_vectors(vector *v1, vector *v2, double *v1_length, double *v2_length)
{
  return acos((v1->x * v2->x + v1->y * v2->y) / (*v1_length * *v2_length));
}

// https://forum.unity.com/threads/how-vector2-dot-works-solved.481514/
void vector_and_point_to_distance_and_angle(vector *v, point *p, int *distance, int *angle)
{
  double vector_length = get_vector_length(v);

  vector v_normalize;
  v_normalize.x = v->x / vector_length;
  v_normalize.y = v->y / vector_length;

  vector p_vector;
  p_vector.x = p->x;
  p_vector.y = p->y;

  // https://www.dummies.com/education/science/physics/how-to-find-the-angle-and-magnitude-of-a-vector/
  vector perpendicular_v;
  perpendicular_v.x = v_normalize.y;
  perpendicular_v.y = -v_normalize.x;

  double normalize_length = 1;
  double point_length = get_vector_length(&p_vector);

  double radians = get_radians_between_2_vectors(&v_normalize, &p_vector, &normalize_length, &point_length);

  *distance = (int) ((point_length * normalize_length) * cos(radians));

  *angle = atan2(perpendicular_v.y, perpendicular_v.x) / RADIAN;
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
  printf("For angle_max %d angle_step %d => %d\n", config->angle_max, config->angle_step, get_angle_step_count(config));
  printf("\n");
}

void test_get_votes_array_size(hough_config *config)
{
  printf("Test get angle step count:\n");
  printf("For distance_max %d distance_step %d angle_max %d angle_step %d => %d\n", config->distance_max, config->distance_step, config->angle_max, config->angle_step, get_votes_array_size(config));
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

void test_get_radians_between_2_vectors_unit(double x1, double y1, double x2, double y2)
{
  vector v1, v2;
  v1.x = x1;
  v1.y = y1;
  v2.x = x2;
  v2.y = y2;
  double v1_length = get_vector_length(&v1);
  double v2_length = get_vector_length(&v2);
  double radian = get_radians_between_2_vectors(&v1, &v2, &v1_length, &v2_length);
  double angle = radian / RADIAN;
  printf("For vector (%f, %f)|%f| and (%f, %f)|%f| => radian: %f angle: %f\n", v1.x, v1.y, v1_length, v2.x, v2.y, v2_length, radian, angle);
}

void test_get_radians_between_2_vectors()
{
  printf("Test get angle between 2 vectors (RADIAN %f):\n", RADIAN);

  test_get_radians_between_2_vectors_unit(1, 0, 0, 1);
  test_get_radians_between_2_vectors_unit(0, 1, 1, 0);
  test_get_radians_between_2_vectors_unit(1, 0, -1, 0);

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
  test_get_radians_between_2_vectors();
  test_vector_and_point_to_distance_and_angle();
}
