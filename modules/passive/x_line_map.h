#ifndef _X_LINE_MAP_H_
#define _X_LINE_MAP_H_

#include "pose.h"

/* the map is automatically redrawn only when a new pose is set */

void init_x_line_map(char *svg_filename, int win_width, int win_height);
void shutdown_x_line_map();

void x_line_map_toggle_pose_visible(int visible);
void x_line_map_set_pose(pose_type p);

#endif
