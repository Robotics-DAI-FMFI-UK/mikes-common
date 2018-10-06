#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "filter.h"

#define LINES_SAME_CLUSTER_MAX_DISTANCE_DIFFERENCE 150
#define LINES_SAME_CLUSTER_MAX_ANGLE_DIFFERENCE 15

int compare_lines_dist(const void *ln1, const void *ln2)
{
  line_data *ld1 = (line_data *)ln1;
  line_data *ld2 = (line_data *)ln2;
  return (ld1->line.distance - ld2->line.distance);
}

void create_clusters(lines_data *original_lines, int *clusters, int lines_count)
{
  qsort(original_lines->lines, lines_count, sizeof(line_data), compare_lines_dist);

  for (int i = 0; i < lines_count; i++)
    clusters[i] = i;

  for (int i = 0; i < lines_count; i++)
  {
    int maxdist = original_lines->lines[i].line.distance + LINES_SAME_CLUSTER_MAX_DISTANCE_DIFFERENCE;
    int j = i + 1;
    short alpha = original_lines->lines[i].line.angle;

    while ((j < lines_count) && (original_lines->lines[j].line.distance <= maxdist))
    {
      short beta = original_lines->lines[j].line.angle;
      if (abs(angle_difference(alpha, beta)) < LINES_SAME_CLUSTER_MAX_ANGLE_DIFFERENCE)
        if (clusters[i] != clusters[j])
        {
          if (clusters[i] < clusters[j])
          {
            for (int k = 0; k < lines_count; k++)
              if (clusters[k] == clusters[j]) clusters[k] = clusters[i];
          }
          else
          {
            for (int k = 0; k < lines_count; k++)
              if (clusters[k] == clusters[i]) clusters[k] = clusters[j];
          }
        }
      j++;
    }
  }
}

int create_avarage_clusters(lines_data *original_lines, int *clusters, uint8_t *cluster_used, int *cluster_total_votes, double *clusters_distance, double *clusters_angle, int lines_count)
{
  int cluster_count = 0;

  int cluster_size[lines_count];
  int cluster_australia[lines_count];

  memset(cluster_used, 0, lines_count);
  memset(cluster_size, 0, sizeof(int) * lines_count);
  memset(cluster_total_votes, 0, sizeof(int) * lines_count);

  // Count all votes for clusters
  for (int i = 0; i < lines_count; i++)
    cluster_total_votes[clusters[i]] += original_lines->lines[i].votes;

  // Create avarage values
  for (int i = 0; i < lines_count; i++)
  {
    if (!cluster_used[clusters[i]])
    {
      cluster_used[clusters[i]] = 1;
      clusters_distance[clusters[i]] = 0.0;
      clusters_angle[clusters[i]] = 0.0;
      cluster_australia[clusters[i]] = normAlpha(original_lines->lines[i].line.angle + 180);
      cluster_count ++;
    }

    cluster_size[clusters[i]]++;
    double line_weight = (original_lines->lines[i].votes / (double) cluster_total_votes[clusters[i]]);
    clusters_distance[clusters[i]] += line_weight * original_lines->lines[i].line.distance;
    int omega = original_lines->lines[i].line.angle;
    if (omega < cluster_australia[clusters[i]]) omega += 360;
    clusters_angle[clusters[i]] += line_weight * omega;
  }

  return cluster_count;
}

void fill_filtered_lines(uint8_t *cluster_used, int *cluster_total_votes, double *clusters_distance, double *clusters_angle, int lines_count, lines_data *filtered_lines)
{
  for (int i = 0, filtered = 0; i < lines_count; i++) {
    if (cluster_used[i]) {
      filtered_lines->lines[filtered].line.distance = clusters_distance[i];
      filtered_lines->lines[filtered].line.angle = normAlpha(clusters_angle[i]);
      filtered_lines->lines[filtered].votes = cluster_total_votes[i];
      filtered++;
    }
  }
}

void filter_lines(lines_data *original_lines, lines_data *filtered_lines)
{
  int lines_count = original_lines->count;

  int clusters[lines_count];
  create_clusters(original_lines, clusters, lines_count);

  uint8_t cluster_used[lines_count];
  int cluster_total_votes[lines_count];
  double clusters_distance[lines_count];
  double clusters_angle[lines_count];

  int cluster_count = create_avarage_clusters(original_lines, clusters, cluster_used, cluster_total_votes, clusters_distance, clusters_angle, lines_count);
  filtered_lines->count = cluster_count;

  fill_filtered_lines(cluster_used, cluster_total_votes, clusters_distance, clusters_angle, lines_count, filtered_lines);
}
