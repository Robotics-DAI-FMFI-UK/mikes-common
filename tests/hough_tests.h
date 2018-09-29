#ifndef __hough_tests_h__
#define __hough_tests_h__

#include "../bites/hough.h"


int get_distance_step_count(hough_config *config);

int get_angle_step_count(hough_config *config);

int get_votes_array_size(hough_config *config);

int get_angle(tim571_status_data *status_data, int index);

int get_index_from_distance_and_angle(hough_config *config, int distance, int angle);

void get_line_data_from_index(hough_config *config, int index, line_data *line);

int is_good_value(hough_config *config, uint16_t distance, uint16_t rssi);

int is_good_candidat(hough_config *config, int number_of_votes);

void angle_to_vector(int angle, vector *v);

void get_perpendicular_vector(vector *v, vector *perpendicular);

void get_point_from_vector_and_distance(vector *v, int *distance, point *p);

void rotate_vector_by_angle(vector *line, int *angle, vector *result);

double get_vector_length(vector *v);

double get_c_from_vector_and_point(vector *normal_v, point *p);

double distance_of_point_from_line(point *p, vector *line_direction_v, point *line_p);

double angle_from_axis_x(vector *v);

void vector_from_two_points(point *p1, point *p2, vector *v);

void find_cross_of_two_lines(vector *normal_v1, point *p1, vector *normal_v2, point *p2, point *result);

void vector_and_point_to_distance_and_angle(vector *line_directional, point *line_p, int *distance, int *angle);

void test_get_distance_step_count(hough_config *config);

void test_get_angle_step_count(hough_config *config);

void test_get_votes_array_size(hough_config *config);

void test_get_angle(tim571_status_data *status_data);

void test_get_index_from_distance_and_angle(hough_config *config);

void test_get_line_data_from_index_unit(hough_config *config, int index, line_data *line);

void test_get_line_data_from_index(hough_config *config);

void test_angle_to_vector_unit(int angle, vector *v);

void test_angle_to_vector();

void test_get_perpendicular_vector_unit(int x, int y);

void test_get_perpendicular_vector();

void test_get_point_from_vector_and_distance_unit(double x, double y, int distance);

void test_get_point_from_vector_and_distance();

void test_rotate_vector_by_angle_unit(double x, double y, int angle);

void test_rotate_vector_by_angle();

void test_get_vector_length_unit(double x, double y);

void test_get_vector_length();

void test_distance_of_point_from_line_unit(double px, double py, double sx, double sy, double lx, double ly);

void test_distance_of_point_from_line();

void test_angle_from_axis_x_unit(double vx, double vy);

void test_angle_from_axis_x();

void test_find_cross_of_two_lines_unit(double v1x, double v1y, double p1x, double p1y, double v2x, double v2y, double p2x, double p2y);

void test_find_cross_of_two_lines();

void test_vector_and_point_to_distance_and_angle_unit(double vx, double vy, double px, double py);

void test_vector_and_point_to_distance_and_angle();

void test_hough_methods(hough_config *config, tim571_status_data *status_data, uint16_t *distance, uint8_t *rssi);

#endif
