#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#include "float.h"

#include "pose.h"
#include "core/config_mikes.h"
#include "../live/base_module.h"
#include "gridmap.h"
#include "mikes_logs.h"

uint32_t **grid_empty;
uint32_t **grid_occupied;

typedef struct cell_struct {
		int parent_x, parent_y;
		double f,g,h;
} cell;

typedef struct node_struct {
	int y,x;
	double f;
	struct node_struct *next;
} node;

node *front = NULL;
node *rear = NULL;

void init_gridmap(){
	grid_empty = (uint32_t **) malloc (mikes_config.gridmap_height * sizeof(uint32_t *));
	grid_occupied = (uint32_t **) malloc (mikes_config.gridmap_height * sizeof(uint32_t *));
	
	for (int i= 0; i< mikes_config.gridmap_height; i++)
	{
		grid_empty[i] = (uint32_t *) malloc (mikes_config.gridmap_width * sizeof(uint32_t));
		grid_occupied[i] = (uint32_t *) malloc (mikes_config.gridmap_width * sizeof(uint32_t));
	}
	
	for (int i= 0; i< mikes_config.gridmap_height; i++)
	{
		for (int j= 0; j< mikes_config.gridmap_width; j++)
		{
			grid_empty[i][j] = 0;
			grid_occupied[i][j] = 0;
		}
	}
	set_pose(mikes_config.gridmap_width/2*10, mikes_config.gridmap_height/2*10, 0);
}

uint8_t cell_valid(int y, int x){
	if (y>=0 && y<mikes_config.gridmap_height && x>=0 && x< mikes_config.gridmap_width)
	{
		return 1;
	}
	return 0;
}

void inc_grid_empty(int y, int x){
	if (cell_valid(y,x))
	{
		grid_empty[y][x]++;
	}
}


void inc_grid_occupied(int y, int x){
	if (cell_valid(y,x))
	{
		grid_occupied[y][x]++;
	}
}

void get_merged_grid(double **gridmap){
	for (int i = 0; i< mikes_config.gridmap_height; i++)
	{
		for (int j= 0; j< mikes_config.gridmap_width; j++)
		{
			gridmap[i][j] = -1; 									// Set all 
			uint32_t total = grid_empty[i][j]+grid_occupied[i][j];
			if (total>0)
			{
				gridmap[i][j] = grid_occupied[i][j]/(double) total; 
			}
		}
	}
}

void get_gridmap_extended_obstacles(double **gridmap, uint16_t robot_radius){
	
	double **new_grid;
	new_grid = (double **) malloc (mikes_config.gridmap_height * sizeof(double *));
	for (int i= 0; i< mikes_config.gridmap_height; i++)
	{
		new_grid[i] = (double *) malloc (mikes_config.gridmap_width * sizeof(double));
	}
	robot_radius = robot_radius/10;
	for (int i = 0; i< mikes_config.gridmap_height; i++)
	{
		for (int j= 0; j< mikes_config.gridmap_width; j++)
		{
			if (gridmap[i][j]>=0.35)
			{
				for (int y = i-robot_radius; y < i+robot_radius; y++)
				{
					for (int x = j - robot_radius; x < j + robot_radius; x++)
					{
						if (cell_valid(y, x))
						{
							new_grid[y][x] = 1; 
						}
					}
				}
			}
			else
			{
				new_grid[i][j] = gridmap[i][j];
			}
		}
	}
	gridmap = new_grid;
}

void get_gridmap_for_navigation(double **gridmap){
	get_merged_grid(gridmap);
	get_gridmap_extended_obstacles(gridmap, (WHEELS_DISTANCE + 150)/10);
}

uint8_t cell_empty(double **gridmap, int y, int x){
	return (gridmap[y][x] < 0.35 && gridmap[y][x] != -1);
}

double calc_h_val(double **gridmap, int y, int x, int dest_y, int dest_x){
	int dist = 20; 
	for (int i = - 1; i < 3; i++){
		for (int j = - 1; j < 3; j++){
			if (i == 0 && 0 == j){
				continue;
			}
			int dx = x + j;
			int dy = y + i;
			for (int a = 0; a < 20; a++){
				if (!cell_valid(dy,dx) || gridmap[dy][dx] > 0.35 || gridmap[dy][dx] < 0 || a == 20-1){
					if (dist>a){
						dist = a;
					}
					break;
				}
			}
		}
	}
	return  (double) sqrt( (y-dest_y) * (y-dest_y) + (x - dest_x) * (x - dest_x) )+(20-dist) ;
}

void enqueue(double f, int y, int x){
	node *nptr = malloc(sizeof(node));
	nptr->f = f;
	nptr->y = y;
	nptr->x = x;
	nptr->next = NULL;
	if (rear == NULL)
	{
		front = nptr;
		rear = nptr;
	}
	else
	{
		rear->next = nptr;
		rear = rear->next;
	}
}

void dequeue(){
	if (front == NULL)
	{
		return;
	}
	node *tmp;
	tmp = front;
	front = front->next;
	free(tmp);
}

path_type *trace_path(cell cells[][mikes_config.gridmap_width], int y, int x){
	path_type *path_res;
	path_res = (path_type *) malloc(sizeof(path_type));
	path_res->path_size = 0;
	front = NULL;
	rear = NULL;
	
	while (!( cells[y][x].parent_y == y && cells[y][x].parent_x == x) )
	{
		enqueue(cells[y][x].g,y,x);
		path_res->path_size++;
		int temp_x = cells[y][x].parent_x;
		y = cells[y][x].parent_y;
		x = temp_x;
	}
	enqueue(cells[y][x].g,y,x);
	path_res->path_size++;
	
	path_res->path = malloc(path_res->path_size * 2 * sizeof(int));
	while (front != NULL)
	{
		(*(path_res->path))[path_res->path_size][0] = front->y;
		(*(path_res->path))[path_res->path_size][1] = front->x;
		dequeue();
	}
	return path_res;
}

void release_path(path_type *path)
{
	free(path->path);
	free(path);
}

path_type *find_path_in_gridmap(int start_y, int start_x,int dest_y,int dest_x){ // A* search
	double **gridmap = (double **) malloc (mikes_config.gridmap_height * sizeof(double *));
	for (int i= 0; i< mikes_config.gridmap_height; i++){
		gridmap[i] = (double *) malloc (mikes_config.gridmap_width * sizeof(double));
	}
	get_gridmap_for_navigation(gridmap);
	if (!cell_valid(start_y, start_x) || 
	    !cell_empty(gridmap, start_y, start_x) || 
	    ((start_y == dest_y) && (start_x == dest_x))) {
		mikes_log(ML_DEBUG, "Path finding: invalid start point");
		return 0;
	}
	if (!cell_valid(dest_y, dest_x) || !cell_empty(gridmap, dest_y, dest_x)){
		mikes_log(ML_DEBUG, "Path finding: invalid destination point");
		return 0;
	}
	uint8_t visited[mikes_config.gridmap_height][mikes_config.gridmap_width];
	cell cells[mikes_config.gridmap_height][mikes_config.gridmap_width];
	for (int i = 0; i < mikes_config.gridmap_height; i++)
	{
		for (int j = 0; j < mikes_config.gridmap_width; j++)
		{
			cells[i][j].parent_x = -1;
			cells[i][j].parent_y = -1;
			cells[i][j].f = DBL_MAX;
			cells[i][j].g = DBL_MAX;
			cells[i][j].h = DBL_MAX;
		}
	}
	int y = start_y; 
	int x = start_x;
	cells[start_y][start_x].f = 0.0;
	cells[start_y][start_x].g = 0.0;
	cells[start_y][start_x].h = 0.0;
	cells[start_y][start_x].parent_x = x;
	cells[start_y][start_x].parent_y = y;
	
	enqueue(0.0,start_y,start_x);
	
	while(front != NULL)
	{
		y = front->y;
		x = front->x;
		visited[y][x] = 1;
		
		uint8_t f_new, g_new, h_new;
		
		for (int i = y - 1; i < y + 2; i++)
		{
			for (int j = x - 1; j < x + 2; j++)
			{
				if ((y == i && j == x ) || !cell_valid(i,j))
				{
					continue;
				}
				if (i == dest_y && j == dest_x) // destination found
				{
					cells[i][j].parent_y = y;
					cells[i][j].parent_x = x;
					return trace_path(cells, dest_y, dest_x);
				}
				else if(!visited[i][j])
				{
					g_new = cells[y][x].g + 1.0; //may be modified for travel cost from cell2another
					h_new = calc_h_val(gridmap,i,j,dest_y,dest_x);
					f_new = g_new + h_new;
					if (cells[i][j].f > f_new)
					{
						enqueue(f_new, i, j);
						cells[i][j].f = f_new;
						cells[i][j].g = g_new;
						cells[i][j].h = h_new;
						cells[i][j].parent_x = x;
						cells[i][j].parent_y = y;
					}
				}
			}
		}
		dequeue();
	}
	return 0;
}


double get_robotangle2mappoint(int x, int y, double heading, int map_x, int map_y){
	int delta_x = map_x - x;
	int delta_y = y - map_y;
	double angle = atan2(delta_x, delta_y);
	return heading + angle;
}






