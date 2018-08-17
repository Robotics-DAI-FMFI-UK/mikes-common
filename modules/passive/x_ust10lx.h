#ifndef _X_UST10LX_H_
#define _X_UST10LX_H_

#include "../live/ust10lx.h"

#define X_UST10LX_WIDTH 600
#define X_UST10LX_HEIGHT 600

void init_x_ust10lx(int max_range_in_mm, int window_update_period_in_ms);
void shutdown_x_ust10lx();

#endif
