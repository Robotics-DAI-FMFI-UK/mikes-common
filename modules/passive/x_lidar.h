#ifndef _X_LIDAR_H_
#define _X_LIDAR_H_

#include "../live/lidar.h"

#define X_LIDAR_WIDTH 600
#define X_LIDAR_HEIGHT 600

void init_x_lidar(int max_range_in_mm, int window_update_period_in_ms);
void shutdown_x_lidar();

#endif
