#ifndef _X_SENSOR_FUSION_H_
#define _X_SENSOR_FUSION_H_

#include "../live/hcsr04.h"

#define X_SENSOR_FUSION_WIDTH 600
#define X_SENSOR_FUSION_HEIGHT 600

void init_x_sensor_fusion(int max_range_in_mm, int window_update_period_in_ms);
void shutdown_x_sensor_fusion();

void sensor_fusion(hcsr04_data_type hcsr04_data_local_copy, uint16_t *dist_local_copy); // TIM571 + ULTRASONIC

#endif
