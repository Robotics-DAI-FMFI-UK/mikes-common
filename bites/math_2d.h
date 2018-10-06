#ifndef _MATH_2D_H_
#define _MATH_2D_H_

#include <math.h>

#define FULL_ANGLE 360
#define HALF_ANGLE 180
#define PERPENDICULAR_ANGLE 90
#define RADIAN (M_PI / HALF_ANGLE)

typedef struct angle_line_struct_2d {
  double angle;
  double distance;
} angle_line_2d;

typedef struct vector_struct_2d {
  double x;
  double y;
} vector_2d;

typedef struct point_struct_2d {
  double x;
  double y;
} point_2d;

void angle_to_vector(double *angle, vector_2d *v);

void get_normal_vector(vector_2d *v, vector_2d *normal);

void vector_and_distance_to_point(vector_2d *v, double *distance, point_2d *p);

void get_line_data_from_distance_and_angle(double *dist, double *ang, vector_2d *v, vector_2d *line_vector, point_2d *line_point);

void rotate_vector_by_angle(vector_2d *line, int *angle, vector_2d *result);

double get_vector_length(vector_2d *v);

void normalize_vector(vector_2d *v);

double get_offset_from_vector_and_point(vector_2d *normal_v, point_2d *p);

double angle_between_vectors(vector_2d *v1, vector_2d *v2);

double angle_from_axis_x(vector_2d *v);

void vector_from_two_points(point_2d *p1, point_2d *p2, vector_2d *v);

void find_cross_of_two_lines(vector_2d *normal_v1, point_2d *p1, vector_2d *normal_v2, point_2d *p2, point_2d *result);

void find_distance_and_angle_between_point_and_line(point_2d *point, vector_2d *line_directional, point_2d *line_p, double *distance, double *angle);

#endif
