#include <stdio.h>
#include "hough_tests.h"

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
