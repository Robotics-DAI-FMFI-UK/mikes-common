#ifndef _MAPPING_NAVIG_H_
#define _MAPPING_NAVIG_H_

#include <stdint.h>

#define MAX_TRAJECTORY_SIZE 1000000

extern int32_t robot_trajectory[MAX_TRAJECTORY_SIZE][2];
extern uint32_t trajectory_size;

void init_mapping_navig();
void shutdown_mapping_navig();
void pause_mapping_navig(uint8_t value);

#endif
