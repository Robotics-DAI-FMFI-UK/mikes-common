#ifndef _GRIDMAP_H_
#define _GRIDMAP_H_

#include <inttypes.h>

extern uint32_t **grid_empty;
extern uint32_t **grid_occupied;

typedef struct path_struct{
	int path_size;
	int (*path)[][2];	
} path_type;

void init_gridmap();
void inc_grid_empty(int y, int x);
void inc_grid_occupied(int y, int x);

path_type *find_path_in_gridmap(int start_x, int start_y,int dest_x,int dest_y);

#endif
