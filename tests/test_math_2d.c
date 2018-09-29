#include <stdio.h>
#include <stdlib.h>
#include "../bites/math_2d.h"

void test_angle_to_vector_unit(double angle, vector_2d *v)
{
  angle_to_vector(&angle, v);
  printf("For angle %f => x %f y %f\n", angle, v->x, v->y);
}

void test_angle_to_vector()
{
  printf("Test angle to vector:\n");
  vector_2d v;

  test_angle_to_vector_unit(0, &v);
  test_angle_to_vector_unit(45, &v);
  test_angle_to_vector_unit(90, &v);
  test_angle_to_vector_unit(180, &v);
  test_angle_to_vector_unit(270, &v);

  printf("\n");
}

void test_get_normal_vector_unit(int x, int y)
{
  vector_2d v, perpendicular;
  v.x = x;
  v.y = y;
  get_normal_vector(&v, &perpendicular);
  printf("For vector (%f, %f) => (%f, %f)\n", v.x, v.y, perpendicular.x, perpendicular.y);
}

void test_get_normal_vector()
{
  printf("Test get perpendicular vector:\n");

  test_get_normal_vector_unit(1, 0);
  test_get_normal_vector_unit(0, -1);

  printf("\n");
}

void test_vector_and_distance_to_point_unit(double x, double y, double distance)
{
  vector_2d v;
  point_2d p;
  v.x = x;
  v.y = y;
  vector_and_distance_to_point(&v, &distance, &p);
  printf("For vector (%f, %f) distance %f => (%f, %f)\n", v.x, v.y, distance, p.x, p.y);
}

void test_vector_and_distance_to_point()
{
  printf("Test get point from vector and distance:\n");

  test_vector_and_distance_to_point_unit(1, 0, 100);
  test_vector_and_distance_to_point_unit(0, -1, 100);
  test_vector_and_distance_to_point_unit(0.707107, 0.707107, 100);

  printf("\n");
}

void test_rotate_vector_by_angle_unit(double x, double y, int angle)
{
  vector_2d v, r;
  v.x = x;
  v.y = y;
  rotate_vector_by_angle(&v, &angle, &r);
  printf("For vector_2d (%f, %f) angle %d => (%f, %f)\n", v.x, v.y, angle, r.x, r.y);
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
  vector_2d v;
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

void test_angle_from_axis_x_unit(double vx, double vy)
{
  vector_2d v;
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
  vector_2d v1, v2;
  point_2d p1, p2, cross;
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

void test_find_distance_and_angle_between_point_and_line_unit(double vx, double vy, double px, double py)
{
  double distance, angle;

  point_2d entry;
  entry.x = 0;
  entry.y = 0;

  vector_2d v;
  v.x = vx;
  v.y = vy;

  point_2d p;
  p.x = px;
  p.y = py;

  find_distance_and_angle_between_point_and_line(&entry, &v, &p, &distance, &angle);
  printf("For vector (%f, %f) point (%f, %f) => distance %f angle %f\n", v.x, v.y, p.x, p.y, distance, angle);
}

void test_find_distance_and_angle_between_point_and_line()
{
  printf("Test vector and point to distance and angle:\n");

  test_find_distance_and_angle_between_point_and_line_unit(1, 0, 3, 1);
  test_find_distance_and_angle_between_point_and_line_unit(-1, 0, 3, 1);
  test_find_distance_and_angle_between_point_and_line_unit(1, 0, -3, 1);
  test_find_distance_and_angle_between_point_and_line_unit(1, 0, 1, 3);
  test_find_distance_and_angle_between_point_and_line_unit(1, 0, 1, -3);
  test_find_distance_and_angle_between_point_and_line_unit(0, 1, 3, 1);
  test_find_distance_and_angle_between_point_and_line_unit(0, -1, 3, 1);

  printf("\n");
}

int main(int argc,char **argv)
{
  test_angle_to_vector();
  test_get_normal_vector();
  test_vector_and_distance_to_point();
  test_rotate_vector_by_angle();
  test_get_vector_length();
  test_angle_from_axis_x();
  test_find_cross_of_two_lines();
  test_find_distance_and_angle_between_point_and_line();

  exit(0);
}
