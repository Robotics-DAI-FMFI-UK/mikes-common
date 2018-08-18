#ifndef _X_LIDAR_H_
#define _X_LIDAR_H_

#include "../live/xtion/xtion.h"

void init_x_xtion(int width, int height, double zoom, int window_update_period_in_ms);
void shutdown_x_xtion();

#endif
