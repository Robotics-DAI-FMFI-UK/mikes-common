#ifndef _X_BASE_H_
#define _X_BASE_H_

#define X_BASE_WIDTH 250
#define X_BASE_HEIGHT 250

#define AZIMUTH_NOT_SET 100000

void init_x_base(int window_update_period_in_ms);
void shutdown_x_base();

void x_base_set_azimuth(double new_azimuth);


#endif
