#include "math_2d.h"

void angle_to_vector(double *angle, vector_2d *v)
{
  double rad = RADIAN * *angle;
  v->x = cos(rad);
  v->y = sin(rad);
}

void get_normal_vector(vector_2d *v, vector_2d *normal)
{
  normal->x = v->y;
  normal->y = -v->x;
}

void vector_and_distance_to_point(vector_2d *v, double *distance, point_2d *p)
{
  p->x = v->x * *distance;
  p->y = v->y * *distance;
}

void get_line_data_from_distance_and_angle(double *dist, double *ang, vector_2d *v, vector_2d *line_vector, point_2d *line_point)
{
  angle_to_vector(ang, v);
  get_normal_vector(v, line_vector);
  vector_and_distance_to_point(v, dist, line_point);
}

void rotate_vector_by_angle(vector_2d *line, int *angle, vector_2d *result)
{
  double rad = RADIAN * *angle;
  double cosRad = cos(rad);
  double sinRad = sin(rad);
  result->x = cosRad * line->x - sinRad * line->y;
  result->y = sinRad * line->x + cosRad * line->y;
}

double get_vector_length(vector_2d *v)
{
  return sqrt(v->x * v->x + v->y * v->y);
}

void normalize_vector(vector_2d *v)
{
  double length = get_vector_length(v);
  v->x = v->x / length;
  v->y = v->y / length;
}

double get_offset_from_vector_and_point(vector_2d *normal_v, point_2d *p)
{
  return -(normal_v->x * p->x + normal_v->y *p->y);
}

double angle_between_vectors(vector_2d *v1, vector_2d *v2)
{
  double dot = v1->x * v2->x + v1->y * v2->y;
  double det = v1->x * v2->y - v1->y * v2->x;
  return atan2(det, dot) / RADIAN;
}

double angle_from_axis_x(vector_2d *v)
{
  return atan2(v->y, v->x) / RADIAN;
}

void vector_from_two_points(point_2d *p1, point_2d *p2, vector_2d *v)
{
  v->x = p2->x - p1->x;
  v->y = p2->y - p1->y;
}

void find_cross_of_two_lines(vector_2d *normal_v1, point_2d *p1, vector_2d *normal_v2, point_2d *p2, point_2d *result)
{
  double c1 = get_offset_from_vector_and_point(normal_v1, p1);
  double c2 = get_offset_from_vector_and_point(normal_v2, p2);

  if (normal_v2->x != 0) { // normal_v2.x cant be 0
    result->y = ((c2 * normal_v1->x) - (c1 * normal_v2->x)) / ((normal_v1->y * normal_v2->x) - (normal_v1->x * normal_v2->y));
    result->x = -(normal_v2->y * result->y + c2) / normal_v2->x;
  } else { // normal_v2.y cant be 0
    result->x = ((c2 * normal_v1->y) - (c1 * normal_v2->y)) / ((normal_v1->x * normal_v2->y) - (normal_v1->y * normal_v2->x));
    result->y = -(normal_v2->x * result->x + c2) / normal_v2->y;
  }
}

void find_distance_and_angle_between_point_and_line(point_2d *point, vector_2d *line_directional, point_2d *line_p, double *distance, double *angle)
{
  vector_2d perpendicular_v;
  get_normal_vector(line_directional, &perpendicular_v);

  point_2d cross;
  find_cross_of_two_lines(&perpendicular_v, line_p, line_directional, point, &cross);

  vector_2d cross_v;
  vector_from_two_points(point, &cross, &cross_v);

  *distance = get_vector_length(&cross_v);

  double result_angle;
  if (cross_v.x == 0 && cross_v.y == 0) {
    result_angle = angle_from_axis_x(line_directional) + 90;
  } else {
    result_angle = angle_from_axis_x(&cross_v);
  }

  while (result_angle < 0) {
    result_angle += FULL_ANGLE;
  }

  while (result_angle > FULL_ANGLE) {
    result_angle -= FULL_ANGLE;
  }

  *angle = result_angle;
}
