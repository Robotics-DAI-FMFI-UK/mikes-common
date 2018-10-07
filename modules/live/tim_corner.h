#ifndef _TIM_CORNER_H_
#define _TIM_CORNER_H_

#include "../../bites/corner.h"

typedef void (*tim_corner_receive_data_callback)(corners_data *corners);

void init_tim_corner();
void shutdown_tim_corner();

void tim_corner_change_default_config(corner_config *config);

void register_tim_corner_callback(tim_corner_receive_data_callback callback);
void unregister_tim_corner_callback(tim_corner_receive_data_callback callback);

#endif
