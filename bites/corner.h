#ifndef _CORNER_H_
#define _CORNER_H_

#include "math_2d.h"
#include "segment.h"

#define CORNER_ANGLE_LINE_COUNT 2
#define CORNERS_MAX_DATA_COUNT 5

typedef struct corner_config_struct {
  double max_tolerance_angle;
  double max_tolerance_distance;
} corner_config;

typedef struct corner_struct {
  point_2d corner;
  segment_data segment1;
  segment_data segment2;
} corner_data;

typedef struct corners_struct {
  corner_data corners[CORNERS_MAX_DATA_COUNT];
  int count;
} corners_data;

void corner_find_from_segments(corner_config *config, segments_data *segments, corners_data *corners);
void corner_print_data(corners_data *corners);

#endif
