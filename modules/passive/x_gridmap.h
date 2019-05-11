#ifndef _X_GRIDMAP_H_
#define _X_GRIDMAP_H_

#include "pose.h"

/* the map is automatically redrawn only when a new pose is set */

void init_x_gridmap(int win_width, int win_height, int window_update_period_in_ms);
void shutdown_x_gridmap();

void x_gridmap_toggle_pose_visible(int visible);

#endif
