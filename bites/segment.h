#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include "hough.h"

#define SEGMENTS_MAX_DATA_COUNT 200

typedef struct segment_struct {
  point_2d start;
  point_2d end;
  angle_line_2d line;
  int votes;
  double length;
} segment_data;

typedef struct segments_struct {
  segment_data segments[SEGMENTS_MAX_DATA_COUNT];
  int count;
} segments_data;

typedef struct segment_config_struct {
  double max_distance_error; // 10.0
  int min_points_segment; // 5
  int max_points_skip; // 5
  uint8_t bad_rssi; // 0
} segment_config;

void segment_transform_points_and_lines_to_segments(segment_config *config, tim571_status_data *status_data, uint16_t *distance, uint8_t *rssi, lines_data *lines, segments_data *segments);
void segment_print_segments_data(segments_data *data);

#endif
