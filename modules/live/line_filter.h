#ifndef __line_filter_h__
#define __line_filter_h__

#include "tim_hough_transform.h"

typedef void (*line_filter_callback)(lines_data *lines);

void init_line_filter();

void shutdown_line_filter();

void register_line_filter_callback(line_filter_callback callback);

void unregister_line_filter_callback(line_filter_callback callback);

#endif
