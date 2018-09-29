#ifndef _HOUGH_H_
#define _HOUGH_H_

#include "../modules/live/tim571.h"
#include "math_2d.h"

#define LINE_MAX_DATA_COUNT 50

typedef struct line_struct {
  int distance;
  int angle;     // 0 angle is on the right side of the robot, increasing counter-clockwise
  int votes;
} line_data;

typedef struct lines_struct {
  line_data lines[LINE_MAX_DATA_COUNT];
  int line_count;
} lines_data;

typedef struct hough_config_struct {
  int distance_max; // 15000
  int distance_step; // 15

  int angle_step; // 5

  int votes_min; // 10

  int bad_distance; // 0
  int bad_rssi; // 0
} hough_config;

void hough_get_lines_data(hough_config *config, tim571_status_data *status_data, uint16_t *distance, uint8_t *rssi, lines_data *data);
void hough_print_lines_data(lines_data *data);

#endif
