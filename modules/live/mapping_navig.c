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
#include "../passive/x_gridmap.h"
#include "base_module.h"
#include "gridmapping.h"
#include "tim571.h"
#include "t265.h"

#define ARC_DIST 1500 
#define MIN_ARC_ANGLE 30
#define MAX_RAY_DIST 5000
#define US_COUNT 8

double tim_arc_angle; //angle to check in distance in target direction

static double angle_tolerance;

static uint16_t             dist_local_copy[TIM571_DATA_COUNT];
static uint8_t              rssi_local_copy[TIM571_DATA_COUNT];
static uint16_t             gauss_dist[TIM571_DATA_COUNT];

static uint16_t				us_data_local_copy[US_COUNT];

static volatile double pose_x;
static volatile double pose_y;
static volatile double heading;

static volatile uint8_t  need_new_data;
static volatile uint8_t  need_new_pos;
static uint8_t  paused_navig;

static int                  fd[2];


double target_heading;
int target_distance = 20;
int traveled_distance;
double tim_scan_pose_x;
double tim_scan_pose_y;


void gaussian_filter(uint16_t *gauss);
void find_arcs(uint16_t *dist, uint16_t arcs[][2], int *arcs_size, int *widest_arc_idx);


void process_navigation(){
	gaussian_filter(gauss_dist);
	
	uint16_t arcs[TIM571_DATA_COUNT][2];
	int arcs_size = 0;
	int widest_arc_idx = -1;
	find_arcs(gauss_dist, arcs, &arcs_size, &widest_arc_idx);
	//target_heading = choose_best_dir(&arcs, &arcs_size);
	int angle;
	if (widest_arc_idx == -1) // no arc found
	{
		//turn around and go back( or reverse)
		angle = 180;
		mikes_log_val(ML_INFO, "NO ARC FOUND", angle);

	}
	else{
		int middle_ray = arcs[widest_arc_idx][0]+(arcs[widest_arc_idx][1]-arcs[widest_arc_idx][0])/2;
		mikes_log_val(ML_INFO, "middle_ray", middle_ray);
		angle = (int)(0.5 + tim571_ray2azimuth(middle_ray));	
		mikes_log_val(ML_INFO, "middle_ray", angle);

		//if (angle<0) angle+=360; 
	}
	target_heading = angle / 180.0 * M_PI - heading;
}

void process_movement(){
		char logstr[100];
		sprintf(logstr, "process_movement: target %.3lf head %.3lf\n", target_heading / M_PI * 180, heading / M_PI * 180);
		mikes_log(ML_INFO, logstr);

	if (heading<target_heading+angle_tolerance && heading > target_heading - angle_tolerance)
	{
		//set_motor_speeds(0,0);
		//perform map scan

		double heading_dif = target_heading-heading;
		int turn_motor_speed = 3 + 3 * ((angle_tolerance - fabs(heading_dif)) / angle_tolerance);
		if (heading_dif < 0){
			set_motor_speeds(turn_motor_speed,6);
		}
		else{
			set_motor_speeds(6,turn_motor_speed);
		}
	}
	else{
		if (target_heading < heading){
			set_motor_speeds(6, -6);
		}
		else{
			set_motor_speeds(-6, 6);
		}
	}
	need_new_pos = 1;
}

uint8_t check_obstacles()
{
	double angle = tim_arc_angle / M_PI * 180;
	int ray_start = tim571_azimuth2ray(angle / 2);
	int ray_end = tim571_azimuth2ray(- angle / 2);
	
	for (; ray_start < ray_end; ray_start++)
	{
		if (dist_local_copy[ray_start] < target_distance){
			
			return 0;
		}
	}
	return 1;
}


void find_arcs(uint16_t *dist, uint16_t arcs[][2], int *arcs_size, int *widest_arc_idx)
{
	uint8_t inside_arc = 0;
	uint16_t idx = 0;
	uint16_t best_arc_size = 0;
	
	for(int i = 0; i < TIM571_DATA_COUNT; i++)
	{
		if (inside_arc)
		{
			if (dist[i] < ARC_DIST)
			{
				if (i - arcs[idx][0] >= MIN_ARC_ANGLE*3)
				{
					if(i-arcs[idx][0] > best_arc_size)
					{
						best_arc_size = i - arcs[idx][0];
						*widest_arc_idx = idx;
					}
					arcs[idx][1] = i - 1;
					idx++;
				}
				inside_arc = 0;
			}
		}
		else
		{
			if (dist[i] >= ARC_DIST)
			{
				arcs[idx][0] = i;
				inside_arc = 1;
			}
		}
	}
	*arcs_size = idx;
	mikes_log_val2(ML_INFO, "arcs: ", *arcs_size, best_arc_size);
}

int choose_best_dir(uint16_t *arcs, int *arcs_size)
{
	
  return 0;	
}


void gaussian_filter(uint16_t *gauss){// TODO: take in account ray intensity (rssi values example needed)
	for (int i=0;i<TIM571_DATA_COUNT;i++)
	{
		if (dist_local_copy[i]<=MAX_RAY_DIST) // && rssi_local_copy[i] >= MIN_RAY_INTENSITY)
		{
			gauss[i]=dist_local_copy[i];
			continue; 		// skip smoothing if ray not satisfactional
		}
		double w[] = {1,4,6,4,1};
		int res = 0;
		int res_w = 0;
		int j = -2;
		int limit = 3;
		if (i<2){ j = 0-i; }
		if (i>TIM571_DATA_COUNT-3){	limit = TIM571_DATA_COUNT - i; }
		
		for (;j<limit;j++)
		{
			if (dist_local_copy[i+j]<=MAX_RAY_DIST) // && rssi_local_copy[i+j] >= MIN_RAY_INTENSITY)
                        {
				//w[j+2] *= rssi_local_copy[i+j]; 
				res += w[j+2]*dist_local_copy[i+j];
				res_w += w[j+2]; 
			}
		}
		if (res_w != 0)
		{	
			gauss[i] = res/res_w;
		}
		else
		{
			gauss[i] = dist_local_copy[i];	
			mikes_log(ML_INFO, "mapping_navig gaussian_filter res_w = 0");
		}
	}
}

double get_arc_angle_in_dist(double width, double distance)
{
	double angle = atan2(width/2,distance);
	return angle*2;
}

void tim571_newdata_callback(uint16_t *dist, uint8_t *rssi, tim571_status_data *status_data)
{
  if (need_new_data)
  {
    need_new_data = 0;
    memcpy(dist_local_copy, dist, sizeof(uint16_t) * TIM571_DATA_COUNT);
    memcpy(rssi_local_copy, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
    alert_new_data(fd);
    if (traveled_distance > target_distance)
    {
		tim_scan_pose_x = pose_x;
		tim_scan_pose_y = pose_y;
	}
  }
}

void t265_newpos_callback(t265_pose_type *pose, double *new_heading)
{
  if (need_new_pos)
  {
	need_new_pos = 0;
	pose_x = pose->translation.x*100;
	pose_y = pose->translation.z*100; //?neg z value
	heading = *new_heading;
	mikes_log_double(ML_INFO, "t265 x", pose_x);
	mikes_log_double(ML_INFO, "t265 y", pose_y);
	mikes_log_double(ML_INFO, "t265 h", *new_heading);
	
  }
}


void *mapping_navig_thread(void *args)
{
  int first_navigation = 1;
  while (program_runs)
  {
    if (wait_for_new_data(fd) < 0) {
      perror("mikes:mapping_navig");
      mikes_log(ML_ERR, "mapping_navig error during waiting on new Data.");
      continue;
    }
    if (!paused_navig){
		traveled_distance = sqrt((pose_x-tim_scan_pose_x)*(pose_x-tim_scan_pose_x)+(pose_y-tim_scan_pose_y)*(pose_y-tim_scan_pose_y));
		need_new_data = 1;
		need_new_pos = 1;
		//int chk_obst = check_obstacles();
		int chk_obst = 1;
		
		mikes_log_val(ML_INFO, "mapping_navig obs", chk_obst);
			
		if (first_navigation || (traveled_distance > target_distance && chk_obst))
		{ //make a new scan
			mikes_log(ML_INFO, "mikes:mapping_navig newscan");
			first_navigation = 0;

			start_scanning();
			process_navigation();	
		}
		else			//continue target heading
		{
			process_movement();
		}
		//x_gridmap_pose_changed();
		need_new_data = 1;
		need_new_pos = 1;
	}
  }

  mikes_log(ML_INFO, "mapping_navig quits.");
  threads_running_add(-1);
  return 0;
}

void init_mapping_navig(){

  need_new_data = 1;
  need_new_pos = 1;
  target_heading = 0;
  angle_tolerance = 10.0 / 180.0 * M_PI;
  tim_arc_angle = get_arc_angle_in_dist(WHEEL_DIAMETER_IN_MM + 150,target_distance);
  pthread_t t;
  
  if (pipe(fd) != 0)
  {
    perror("mikes:mapping_navig");
    mikes_log(ML_ERR, "creating pipe for mapping_navig");
    return;
  }
  
  register_tim571_callback(tim571_newdata_callback);
  register_t265_callback(t265_newpos_callback);
  if (pthread_create(&t, 0, mapping_navig_thread, 0) != 0)
  {
    perror("mikes:mapping_navig");
    mikes_log(ML_ERR, "creating thread for mapping_navig");
  }
  else threads_running_add(1);
  mikes_log(ML_INFO, "mikes:mapping_navig initialized");
}

void shutdown_mapping_navig()
{
  close(fd[0]);
  close(fd[1]);
}

void pause_mapping_navig(uint8_t value){
	paused_navig = value;
	need_new_data = value;
	need_new_pos = value;
}
