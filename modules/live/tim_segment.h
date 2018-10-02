#ifndef _TIM_SEGMENT_H_
#define _TIM_SEGMENT_H_

#include "../../bites/segment.h"

typedef void (*tim_segment_receive_data_callback)(segments_data *segments);

void init_tim_segment();
void shutdown_tim_segment();

void tim_segment_change_default_config(segment_config *config);

void register_tim_segment_callback(tim_segment_receive_data_callback callback);
void unregister_tim_segment_callback(tim_segment_receive_data_callback callback);

#endif
