#ifndef _HOUGH_TRANSFORM_H_
#define _HOUGH_TRANSFORM_H_

#include "../../bites/hough.h"

typedef void (*hough_transform_receive_data_callback)(lines_data *lines);

void init_hough_transform();

void change_default_config(hough_config *config);

void register_hough_transform_callback(hough_transform_receive_data_callback callback);
void unregister_hough_transform_callback(hough_transform_receive_data_callback callback);

#endif
