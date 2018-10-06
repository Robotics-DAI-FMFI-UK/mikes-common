#include <stdio.h>

#include "corner.h"

#define MAX_TOLERANCE_ANGLE 5.0
#define MAX_TOLERANCE_DISTANCE 20.0

uint8_t is_point_close_to_segment(point_2d *point, segment_data *segment)
{
  if (segment->start.x < segment->end.x) {
    return point->x > segment->start.x - MAX_TOLERANCE_DISTANCE && point->x < segment->end.x + MAX_TOLERANCE_DISTANCE;
  }
  return point->x > segment->end.x - MAX_TOLERANCE_DISTANCE && point->x < segment->start.x + MAX_TOLERANCE_DISTANCE;
}

void corner_find_from_segments(segments_data *segments, corners_data *corners)
{
  corners->count = 0;
  point_2d cross;

  double angle, difference;
  segment_data *segment1, *segment2;
  vector_2d segment_v1, segment_v2, segment_normal1, segment_normal2;

  for (int segment_i2 = 1; segment_i2 < segments->count; segment_i2++) {
    segment2 = &segments->segments[segment_i2];
    for (int segment_i1 = 0; segment_i1 < segment_i2; segment_i1++) {
      segment1 = &segments->segments[segment_i1];

      vector_from_two_points(&segment1->start, &segment1->end, &segment_v1);
      vector_from_two_points(&segment2->start, &segment2->end, &segment_v2);
      angle = angle_between_vectors(&segment_v1, &segment_v2);

      while (angle < 0) {
        angle += HALF_ANGLE;
      }

      while (angle > HALF_ANGLE) {
        angle -= HALF_ANGLE;
      }


      difference = PERPENDICULAR_ANGLE - angle;
      difference = difference < 0 ? -difference : difference;

      if (difference < MAX_TOLERANCE_ANGLE) {
        get_normal_vector(&segment_v1, &segment_normal1);
        get_normal_vector(&segment_v2, &segment_normal2);
        normalize_vector(&segment_normal1);
        normalize_vector(&segment_normal2);

        find_cross_of_two_lines(&segment_normal1, &segment1->start, &segment_normal2, &segment2->start, &cross);
        // printf("Cross [%f; %f]\n", cross.x, cross.y);
        // printf("Segment1 [%f; %f] : [%f; %f]\n", segment1->start.x, segment1->start.y, segment1->end.x, segment1->end.y);
        // printf("Segment2 [%f; %f] : [%f; %f]\n", segment2->start.x, segment2->start.y, segment2->end.x, segment1->end.y);
        //
        // printf("Compare %10.4f %10.4f %10.4f;\n", cross.x, segment1->start.x, segment1->end.x);

        if (is_point_close_to_segment(&cross, segment1) && is_point_close_to_segment(&cross, segment2)) {
          if (corners->count < CORNERS_MAX_DATA_COUNT) {
            corners->corners[corners->count++] = cross;
          } else {
            printf("Too much corners\n");
          }
        }
      }
    }
  }
}

void corner_print_data(corners_data *data)
{
  printf("CornersData:\n");
  for (int index = 0; index < data->count; index++) {
    printf("%d: Point [%12.4f; %12.4f]\n", index, data->corners[index].x, data->corners[index].y);
  }
}
