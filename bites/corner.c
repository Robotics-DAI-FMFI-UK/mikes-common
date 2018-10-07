#include <stdio.h>

#include "corner.h"

uint8_t is_point_close_to_segment(double *max_tolerance_distance, point_2d *point, segment_data *segment)
{
  if (segment->start.x < segment->end.x) {
    return point->x > segment->start.x - *max_tolerance_distance && point->x < segment->end.x + *max_tolerance_distance;
  }
  return point->x > segment->end.x - *max_tolerance_distance && point->x < segment->start.x + *max_tolerance_distance;
}

void corner_find_from_segments(corner_config *config, segments_data *segments, corners_data *corners)
{
  corners->count = 0;
  point_2d cross;
  corner_data *corner;

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

      if (difference < config->max_tolerance_angle) {
        get_normal_vector(&segment_v1, &segment_normal1);
        get_normal_vector(&segment_v2, &segment_normal2);
        normalize_vector(&segment_normal1);
        normalize_vector(&segment_normal2);

        find_cross_of_two_lines(&segment_normal1, &segment1->start, &segment_normal2, &segment2->start, &cross);

        if (is_point_close_to_segment(&config->max_tolerance_distance, &cross, segment1) && is_point_close_to_segment(&config->max_tolerance_distance, &cross, segment2)) {
          if (corners->count < CORNERS_MAX_DATA_COUNT) {
            corner = &corners->corners[corners->count++];
            corner->corner = cross;
            corner->segment1 = *segment1;
            corner->segment2 = *segment2;
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
    printf("%d: Point [%12.4f; %12.4f] Line1: (angle - %12.4f°, distance - %12.4f) Line2: (angle - %12.4f°, distance - %12.4f)\n", index, data->corners[index].corner.x, data->corners[index].corner.y, data->corners[index].segment1.line.angle, data->corners[index].segment1.line.distance, data->corners[index].segment2.line.angle, data->corners[index].segment2.line.distance);
  }
}
