#include <stdlib.h>

#include "pose.h"
#include "core/config_mikes.h"
#include "gridmap.h"

uint32_t **grid_empty;
uint32_t **grid_occupied;

void init_gridmap(){
	grid_empty = (uint32_t **) malloc (mikes_config.gridmap_height * sizeof(uint32_t *));
	grid_occupied = (uint32_t **) malloc (mikes_config.gridmap_height * sizeof(uint32_t *));
	
	for (int i= 0; i< mikes_config.gridmap_height; i++){
		grid_empty[i] = (uint32_t *) malloc (mikes_config.gridmap_width * sizeof(uint32_t));
		grid_occupied[i] = (uint32_t *) malloc (mikes_config.gridmap_width * sizeof(uint32_t));
	}
	
	for (int i= 0; i< mikes_config.gridmap_height; i++){
		for (int j= 0; j< mikes_config.gridmap_width; j++){
			grid_empty[i][j] = 0;
			grid_occupied[i][j] = 0;
		}
	}
	set_pose(mikes_config.gridmap_width/2*10, mikes_config.gridmap_height/2*10, 0);
}

void inc_grid_empty(int y, int x){
	if (y>=0 && y<mikes_config.gridmap_height && x>=0 && x< mikes_config.gridmap_width){
		grid_empty[y][x]++;
	}
}


void inc_grid_occupied(int y, int x){
	if (y>=0 && y<mikes_config.gridmap_height && x>=0 && x< mikes_config.gridmap_width){
		grid_occupied[y][x]++;
	}
}
