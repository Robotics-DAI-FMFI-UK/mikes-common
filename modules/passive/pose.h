#ifndef _POSE_H_
#define _POSE_H_
#include "../mikes-common/modules/live/base_module.h"

// STATE_WIDTH in cm
#define STATE_WIDTH  25    
#define STATES_W ((MAP_W/STATE_WIDTH) + 1)
#define STATES_H ((MAP_H/STATE_WIDTH) + 1)

typedef struct pose_str {
	double x;  // in cm
	double y;  // in cm
	double heading;
        int row;
        int col;
} pose_type;

void init_pose(int init_reverse_y, int init_max_y);
void update_pose(base_data_type *base_data);
void get_pose(pose_type *pose);
void log_pose(pose_type *pose);
void set_pose(double newx, double newy, double newheading);

#endif
