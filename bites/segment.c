#include <stdio.h>
#include <stdlib.h>

#include "segment.h"

int segment_comparator(const void *a, const void *b) {
   return ((*(segment_data*)b).length - (*(segment_data*)a).length);
}

void segment_transform_points_and_lines_to_segments(segment_config *config, tim571_status_data *status_data, uint16_t *distances, uint8_t *rssi, lines_data *lines, segments_data *segments)
{
  line_data *line;
  double line_distance, distance, line_angle, angle;

  vector_2d line_normal, line_vector, point_vector, point_normal;
  point_2d line_point, point;

  vector_2d start_normal, end_normal, segment_vector;
  point_2d start_point, end_point;

  segments->count = 0;
  for (int line_i = 0; line_i < lines->count; line_i++) {
    line = &lines->lines[line_i];
    line_distance = line->line.distance;
    line_angle = line->line.angle;

    get_line_data_from_distance_and_angle(&line_distance, &line_angle, &line_normal, &line_vector, &line_point);

    for (int point_i = 0, bad_in_row = 0, has_start = 0, votes = 0; point_i < status_data->data_count; point_i++) {
      if (rssi[point_i] > config->bad_rssi) {
        distance = distances[point_i];
        angle = get_angle(status_data, point_i);

        get_line_data_from_distance_and_angle(&distance, &angle, &point_vector, &point_normal, &point);
        find_distance_and_angle_between_point_and_line(&point, &line_vector, &line_point, &distance, &angle);

        if (distance <= config->max_distance_error) {
          if (has_start) {
            votes++;
            end_normal = point_normal;
            end_point = point;
          } else {
            has_start = 1;
            votes = 1;
            end_normal = start_normal = point_normal;
            end_point = start_point = point;
          }
        } else if (has_start && (++bad_in_row > config->max_points_skip || point_i + 1 == status_data->data_count)) {
          has_start = 0;

          if (votes > config->min_points_segment) {
            if (segments->count < SEGMENTS_MAX_DATA_COUNT) {
              segment_data *segment = &segments->segments[segments->count++];
              segment->votes = votes;

              segment->line.distance = line_distance;
              segment->line.angle = line_angle;

              find_cross_of_two_lines(&start_normal, &start_point, &line_normal, &line_point, &segment->start);
              find_cross_of_two_lines(&end_normal, &end_point, &line_normal, &line_point, &segment->end);

              vector_from_two_points(&segment->start, &segment->end, &segment_vector);
              segment->length = get_vector_length(&segment_vector);
            } else {
              printf("Too much segments\n");
            }
          }
        }
      }
    }
  }

  qsort(segments->segments, segments->count, sizeof(segment_data), segment_comparator);
}

void segment_print_segments_data(segments_data *data)
{
  printf("SegmentsData:\n");
  for (int index = 0; index < data->count; index++) {
    printf("%d: Start: (%12.4f; %12.4f), End: (%12.4f; %12.4f), Line: (angle - %12.4f°, distance - %12.4f), Votes: %5d, Length: %12.4f\n", index, data->segments[index].start.x, data->segments[index].start.y, data->segments[index].end.x, data->segments[index].end.y, data->segments[index].line.angle, data->segments[index].line.distance, data->segments[index].votes, data->segments[index].length);
  }
}
