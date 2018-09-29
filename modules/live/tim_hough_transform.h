#ifndef _HOUGH_TRANSFORM_H_
#define _HOUGH_TRANSFORM_H_

#include "../../bites/hough.h"

typedef void (*tim_hough_transform_receive_data_callback)(lines_data *lines);

void init_tim_hough_transform();
void shutdown_tim_hough_transform();

void tim_hough_transform_change_default_config(hough_config *config);

void register_tim_hough_transform_callback(tim_hough_transform_receive_data_callback callback);
void unregister_tim_hough_transform_callback(tim_hough_transform_receive_data_callback callback);

#endif
