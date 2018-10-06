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

void test_hough_methods(hough_config *config, tim571_status_data *status_data, uint16_t *distance, uint8_t *rssi)
{
  test_get_distance_step_count(config);
  test_get_angle_step_count(config);
  test_get_votes_array_size(config);
  test_get_angle(status_data);
  test_get_index_from_distance_and_angle(config);
  test_get_line_data_from_index(config);
}
