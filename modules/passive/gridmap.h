#ifndef _GRIDMAP_H_
#define _GRIDMAP_H_

#include <inttypes.h>

extern uint32_t **grid_empty;
extern uint32_t **grid_occupied;

void init_gridmap();
void inc_grid_empty(int y, int x);
void inc_grid_occupied(int y, int x);


#endif
