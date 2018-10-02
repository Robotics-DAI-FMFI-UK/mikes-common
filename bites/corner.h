#ifndef _CORNER_H_
#define _CORNER_H_

#include "math_2d.h"
#include "segment.h"

#define CORNERS_MAX_DATA_COUNT 5

typedef struct corners_struct {
  point_2d corners[CORNERS_MAX_DATA_COUNT];
  int count;
} corners_data;

void corner_find_from_segments(segments_data *segments, corners_data *corners);
void corner_print_data(corners_data *corners);

#endif
