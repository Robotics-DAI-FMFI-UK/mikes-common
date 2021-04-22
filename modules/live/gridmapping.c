#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "core/config_mikes.h"

#include "../mikes-common/bites/mikes.h"
#include "../../bites/util.h"
#include "../passive/mikes_logs.h"
#include "../passive/pose.h"
#include "../passive/gridmap.h"
#include "../passive/x_gridmap.h"
#include "tim571.h"
#include "t265.h"

#define EPSILON 0.000000001

static uint16_t             dist_local_copy[TIM571_DATA_COUNT];
static uint8_t              rssi_local_copy[TIM571_DATA_COUNT];
static t265_pose_type 	pose_local_copy;

static int initial_pose_x;
static int initial_pose_y;
static double heading;

static volatile uint8_t  need_new_data;
static volatile uint8_t  need_new_pos;

static int                  fd[2];

void tim571_newdata_callback(uint16_t *dist, uint8_t *rssi, tim571_status_data *status_data)
{
  if (need_new_data)
  {
    need_new_data = 0;
    memcpy(dist_local_copy, dist, sizeof(uint16_t) * TIM571_DATA_COUNT);
    memcpy(rssi_local_copy, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
    alert_new_data(fd);
    
  }
}

void t265_newpos_callback(t265_pose_type *pose, double *new_heading)
{
  if (need_new_pos)
  {
	need_new_pos = 0;
	pose_local_copy.translation.x = pose->translation.x;
	pose_local_copy.translation.y = pose->translation.y;
	pose_local_copy.translation.z = pose->translation.z;
	heading = *new_heading;
  }
}

int line_cell_intersects(double x, double y, double alpha, double cell_x, double cell_y, double cell_width, double cell_height, uint16_t max_dist){
	double d;
	if (fabs(alpha-M_PI_2) < EPSILON){
		d = fabs(cell_x + cell_width - x);
		if (d > max_dist) return 2;
		if (y >= cell_y && y <= cell_y+cell_height) return 1;
		return 0;
	}
	if ( fabs(alpha-M_PI_2*3) < EPSILON){
		d = fabs(cell_x - x);
		if (d > max_dist) return 2;
		if (y >= cell_y && y <= cell_y+cell_height) return 1;
		return 0;
	}
	if (fabs(alpha-M_PI) < EPSILON ){
		d = fabs(cell_y - y);
		if (d > max_dist) return 2;
		if (x >= cell_x && x <= cell_x+cell_width) return 1;
		return 0;
	}
	if (fabs(alpha) < EPSILON || fabs(alpha - 2*M_PI) < EPSILON){
		d = fabs(cell_y + cell_height - y);
		if (d > max_dist) return 2;
		if (x >= cell_x && x <= cell_x+cell_width) return 1;
		return 0;
	}
	int is_in_range = 0;
	int is_in_cell = 0;

	double max_dist_2 = max_dist * max_dist;
	double y1 = (cell_x - x)/tan(alpha) + y;
		
	if (y1 >= cell_y && y1 <= cell_y+cell_height){
		is_in_cell = 1;
	
		d = (y1 - y)*(y1-y) + (cell_x - x)*(cell_x - x);
		if (d <= max_dist_2) is_in_range = 1;
	}
	double y2 = (cell_x + cell_width - x)/tan(alpha) + y;
	if (y2 >= cell_y && y2 <= cell_y+cell_height){
		is_in_cell = 1;
		if (!is_in_range){
			d = (y2 - y)*(y2-y) + (cell_x + cell_width - x)*(cell_x + cell_width - x);
			if (d <= max_dist_2) is_in_range = 1;
		}	
	}
	
	double x1 = tan(alpha)*(cell_y - y) +x;
	if (x1 >= cell_x && x1 <= cell_x + cell_width){
		is_in_cell = 1;
		if (!is_in_range){
			d = (x1 - x)*(x1-x) + (cell_y - y)*(cell_y - y);
			if (d <= max_dist_2) is_in_range = 1;
		}
	}
	double x2 = tan(alpha)*(cell_y + cell_height - y) +x;
	if (x2 >= cell_x && x2 <= cell_x+cell_width){
		is_in_cell = 1;
		if (!is_in_range){
			d = (x2 - x)*(x2-x) + (cell_y + cell_height - y)*(cell_y + cell_height - y);
			if (d <= max_dist_2) is_in_range = 1;
		}
	}
	if (is_in_cell){
		if (!is_in_range) return 2;
		return 1;
	}
	d = (cell_y - y)*(cell_y - y) + (cell_x - x)*(cell_x - x);
	if (d>= (max_dist + 1.5*cell_width) * (max_dist + 1.5*cell_width)){
		return 2;
	}
	return 0;
}

void put_tim571_data_to_gridmap(){
	
	for (int i= 0; i<TIM571_DATA_COUNT;i++){
		if (dist_local_copy[i]==0) continue;
		double angle = tim571_ray2azimuth(i);
		if (angle<0) angle+=360; 
		double map_angle = angle / 180 * M_PI - heading;
		while (map_angle < 0) map_angle+=2*M_PI;
		while (map_angle > 2*M_PI) map_angle-=2*M_PI;
		int pos_x = initial_pose_x + pose_local_copy.translation.x*100;
		int pos_y = initial_pose_y - pose_local_copy.translation.z*100;
		int x = pos_x;
		int y = pos_y;
		int dx;
		int dy;
		if (map_angle< M_PI_2){
			dx = 10;
			dy = 10;
		}
		else if(map_angle<M_PI){
			dx = 10;
			dy = -10;
		}
		else if (map_angle<M_PI_2*3){
			dx = -10;
			dy = -10;
		}
		else{
			dx = -10;
			dy = 10;
		}			
		
		while(1){
			//int intersects = line_cell_intersects(pos.x, pos.y, map_angle, x + dx, y, 10, 10, dist_local_copy[i] / 10);
			int intersects = line_cell_intersects(pos_x, pos_y, map_angle, x + dx, y, 10, 10, dist_local_copy[i] / 10);
			
			if (intersects == 2) break;
			if (intersects == 1){
				inc_grid_empty(y/10, x/10);
				x += dx;
				continue;
			}
			//intersects = line_cell_intersects(pos.x, pos.y, map_angle, x, y + dy, 10, 10, dist_local_copy[i] / 10);
			intersects = line_cell_intersects(pos_x, pos_y, map_angle, x, y + dy, 10, 10, dist_local_copy[i] / 10);
			if (intersects == 2) break;
			if (intersects == 1){
				inc_grid_empty(y/10,x/10);
				y += dy;
				continue;
			}
			mikes_log(ML_WARN, "gridmapping loosing ray");
			inc_grid_empty(y/10,x/10);
			x+=dx;
			y+=dy;
		}
		inc_grid_occupied(y/10,x/10);
	}
	
}

void *gridmapping_thread(void *args)
{
  while (program_runs)
  {
    if (wait_for_new_data(fd) < 0) {
      perror("mikes:gridmapping");
      mikes_log(ML_ERR, "gridmapping error during waiting on new Data.");
      continue;
    }
    put_tim571_data_to_gridmap();
    x_gridmap_pose_changed();
    //need_new_data = 1;
    //need_new_pos = 1;
  }

  mikes_log(ML_INFO, "gridmapping quits.");
  threads_running_add(-1);
  return 0;
}

void init_gridmapping(){

  need_new_data = 0;
  need_new_pos = 0;
  initial_pose_x = mikes_config.gridmap_width/2*10;
  initial_pose_y = mikes_config.gridmap_height/2*10;
  pthread_t t;
  
  if (pipe(fd) != 0)
  {
    perror("mikes:gridmapping");
    mikes_log(ML_ERR, "creating pipe for gridmapping");
    return;
  }
  
  register_tim571_callback(tim571_newdata_callback);
  register_t265_callback(t265_newpos_callback);
  if (pthread_create(&t, 0, gridmapping_thread, 0) != 0)
  {
    perror("mikes:gridmapping");
    mikes_log(ML_ERR, "creating thread for gridmapping");
  }
  else threads_running_add(1);
}

void shutdown_gridmapping()
{
  close(fd[0]);
  close(fd[1]);
}

void start_scanning(){
    need_new_data = 1;
    need_new_pos = 1;
}
