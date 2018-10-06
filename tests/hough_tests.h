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

void test_hough_methods(hough_config *config, tim571_status_data *status_data, uint16_t *distance, uint8_t *rssi);

#endif
