#ifndef _X_TIM571_H_
#define _X_TIM571_H_

#include "../live/tim571.h"
#include "../live/tim_hough_transform.h"
#include "../live/line_filter.h"

#define X_TIM571_WIDTH 600
#define X_TIM571_HEIGHT 600

extern int show_raw_lines;

void init_x_tim571(int max_range_in_mm, int window_update_period_in_ms);
void shutdown_x_tim571();

#endif
