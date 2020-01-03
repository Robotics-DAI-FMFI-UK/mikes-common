#ifndef _T265_H
#define _T265_H

#include <librealsense2/rs.h>

#include <pthread.h>

typedef rs2_pose t265_pose_type;

typedef void (*t265_receive_data_callback)(t265_pose_type *pose, double *);

void init_t265();

void get_t265_pose(t265_pose_type *pose); // pose values in m

void register_t265_callback(t265_receive_data_callback callback);    // register for getting fresh data after received from sensor (copy quick!)
void unregister_t265_callback(t265_receive_data_callback callback);  // remove previously registered callback
void log_t265_pose(t265_pose_type *pose);

void get_t265_heading(t265_pose_type *pose, double *heading);

#endif
