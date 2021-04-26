#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "core/config_mikes.h"

#include "../mikes-common/bites/mikes.h"
#include "../../bites/util.h"
#include "../passive/mikes_logs.h"
#include "../passive/x_gridmap.h"
#include "base_module.h"
#include "gridmapping.h"
#include "tim571.h"
#include "t265.h"
#include "hcsr04.h"

#define ARC_DIST 2000 
#define MAX_RAY_DIST 5000
#define MAX_NUM_CROSSINGS 50

double min_arc_angle;

double tim_arc_angle; //angle to check in distance in target direction

static double angle_tolerance;

static uint16_t             dist_local_copy[TIM571_DATA_COUNT];
//static uint8_t              rssi_local_copy[TIM571_DATA_COUNT];
static uint16_t             gauss_dist[TIM571_DATA_COUNT];

static hcsr04_data_type1		hcsr04_data_local_copy;

static volatile double pose_x;
static volatile double pose_y;
static volatile double heading;

static volatile uint8_t  need_new_data;
static volatile uint8_t  need_new_pos;
static volatile uint8_t  need_new_hcsr04_data;
static uint8_t  paused_navig;

static int                  fd[2];


double target_heading;
double pref_heading;
int target_distance = 20;
int traveled_distance;
double tim_scan_pose_x;
double tim_scan_pose_y;

int first_navigation;

//crossing variables
uint16_t crossings[MAX_NUM_CROSSINGS];
uint8_t visited[MAX_NUM_CROSSINGS];
double cross_pos[MAX_NUM_CROSSINGS][2];
uint8_t cross_size = 0;
uint8_t in_crossing = 0;



void gaussian_filter(uint16_t *gauss);
void find_arcs(uint16_t *dist, uint16_t arcs[][2], uint8_t *arcs_size);
double choose_best_dir(uint16_t arcs[][2], uint8_t *arcs_size);


void process_navigation(){
	gaussian_filter(gauss_dist);
	
	uint16_t arcs[TIM571_DATA_COUNT][2];
	uint8_t arcs_size = 0;
	find_arcs(gauss_dist, arcs, &arcs_size);
	if (arcs_size > 1)
	{
		process_crossing(arcs, arcs_size);
	}
	target_heading = choose_best_dir(&arcs, &arcs_size);
	mikes_log_double(ML_INFO,"target_heading: ", target_heading);
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
		int turn_motor_speed = 3 + (int)(0.5 + 3 * (fabs(heading_dif)/angle_tolerance));
		mikes_log_int(ML_INFO, "turn motor speed:", turn_motor_speed);
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

void process_crossing(uint16_t arcs[][2], uint8_t *arcs_size)
{
	if (!pref_heading)
	{
		return;
	}
	if (arcs_size == 2)
	{
		continue; //possible false cross when robot facing wall and seeing paths on both sides
	}
	mikes_log(ML_INFO, "Crossing Found");
	if (!in_crossing)
	{
		in_crossing = 1;
		crossings[cross_size]= heading +  M_PI;
		if (heading + M_PI > M_2_PI)
		{
			crossings[cross_size] -= M_2_PI;
		}
		cross_pos[cross_size] = {pose_x, pose_y};
		visited[cross_size] = 1;
		cross_size++;
		for (int i = 0; i < arcs_size; i++)
		{
			int middle_ray = arcs[i][0]+(arcs[i][1]-arcs[i][0])/2;
			crossings[cross_size] = tim571_ray2azimuth(middle_ray)/ 180.0 * M_PI - heading;
			cross_size++;
		}
	} 
	
	
}

uint8_t check_obstacles()
{
	double angle = tim_arc_angle / M_PI * 180;
	int ray_start = tim571_azimuth2ray(angle / 2);
	int ray_end = tim571_azimuth2ray(- angle / 2);
	
	for (; ray_start < ray_end; ray_start++)
	{
		if (gauss_dist[ray_start] < target_distance){
			return 0;
		}
	}
	return 1;
}


void find_arcs(uint16_t *dist, uint16_t arcs[][2], uint8_t *arcs_size)
{
	uint8_t inside_arc = 0;
	uint16_t idx = 0;
	
	for(int i = 0; i < TIM571_DATA_COUNT; i++)
	{
		if (inside_arc)
		{
			if (dist[i] < ARC_DIST)
			{
				if (i - arcs[idx][0] >= min_arc_angle*3C)
				{
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
	mikes_log_val2(ML_INFO, "arcs: ", *arcs_size);
}

double choose_best_dir(uint16_t arcs[][2], uint8_t *arcs_size)
{
	int angle;
	if (arcs_size == 0) // no arc found
	{
		
		if (!check_front_sensors)
		{
			return target_heading;
		}
		//turn around and go back( or reverse)
		angle = 45;
		first_navigation = 1;
		
		mikes_log_val(ML_INFO, "NO ARC FOUND", angle);

	}
	else{
		int best_arc = 0;
		if (pref_heading != -123 && arcs_size>1)
		{
			double h_dif = 10;
			for (int i = 0; i < arcs_size; i++)
			{
				int middle_ray = arcs[i][0]+(arcs[i][1]-arcs[i][0])/2;
				angle = (int)(0.5 + tim571_ray2azimuth(middle_ray));
				if (fabs(pref_heading-(angle / 180.0 * M_PI - heading))< h_dif){
					best_arc = i;
					h_dif = fabs(pref_heading-(angle / 180.0 * M_PI - heading));
				}
			}
		}//TODO: choose best way in crossing
		int middle_ray = arcs[best_arc][0]+(arcs[best_arc][1]-arcs[best_arc][0])/2;
		mikes_log_val(ML_INFO, "best_middle_ray", middle_ray);
		angle = (int)(0.5 + tim571_ray2azimuth(middle_ray));	
		mikes_log_val(ML_INFO, "best_middle_ray angle", angle);
		if (pref_heading == -123){
			pref_heading = angle / 180.0 * M_PI - heading;
		}
		//if (angle<0) angle+=360; 
	}
	return angle / 180.0 * M_PI - heading;	
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
		double res_w = 0;
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
    //memcpy(rssi_local_copy, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
    alert_new_data(fd);
    if (traveled_distance >= target_distance)
    {
		tim_scan_pose_x = pose_x;
		tim_scan_pose_y = pose_y;
	}
	add_ultrasonic_to_tim_data();
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
	mikes_log_double(ML_INFO, "t265 nh", *new_heading);
	mikes_log_double(ML_INFO, "t265 h", heading);
	
  }
}

void hcsr04_newdata_callback(hcsr04_data_type *hcsr04_data)
{
  if (need_new_hcsr04_data)
  {
		need_new_hcsr04_data = 0;
		memcpy(hcsr04_data_local_copy, hcsr04_data, sizeof(uint16_t) * NUM_ULTRASONIC_SENSORS);
		alert_new_data(fd);
  }
}

void add_ultrasonic_to_tim_data()
{//TODO: Use LEFT/RIGHT sensors
	switch(check_front_sensors())
	{
		case 1:
			for (int i = (int)(0.5+TIM571_DATA_COUNT/2 - (min_arc_angle / 2 * 3)); i < (int)(0.5+TIM571_DATA_COUNT/2); i++ )
			{
				dist_local_copy[i] = 1;
			}
			break;
			
		case 2:
			for (int i = (int)(0.5+TIM571_DATA_COUNT/2 - (min_arc_angle / 2 * 3)); i < (int)(0.5+TIM571_DATA_COUNT/2); i++ )
			{
				dist_local_copy[i] = 1;
			}
	}
	
}

uint8_t check_front_sensors(){
	uint8_t min_us_range = 210;
	uint8_t min_bottom_range = 5;
	uint8_t max_bottom_range = 10;
	if (hcsr04_data_local_copy.MIDDLE_LEFT < min_us_range || hcsr04_data_local_copy.TOP_LEFT < min_us_range
	 || hcsr04_data_local_copy.DOWN_LEFT < min_bottom_range || hcsr04_data_local_copy.DOWN_LEFT > max_bottom_range)
	{
		return 1;
	}
	if (hcsr04_data_local_copy.MIDDLE_RIGHT < min_us_range || hcsr04_data_local_copy.TOP_RIGHT < min_us_range
	 || hcsr04_data_local_copy.DOWN_RIGHT < min_bottom_range || hcsr04_data_local_copy.DOWN_RIGHT > max_bottom_range)
	{
		return 2;
	}
}


void *mapping_navig_thread(void *args)
{
  first_navigation = 1;
  in_crossing = 0;
  usleep(1500000L);
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
			
		if (first_navigation || (traveled_distance >= target_distance && chk_obst))
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
  need_new_hcsr04_data = 1;
  target_heading = 0;
  pref_heading = -123.0;
  angle_tolerance = 10.0 / 180.0 * M_PI;
  tim_arc_angle = get_arc_angle_in_dist(WHEEL_DIAMETER_IN_MM + 150,target_distance);
  min_arc_angle = get_arc_angle_in_dist(WHEEL_DIAMETER_IN_MM + 150, ARC_DIST);
  pthread_t t;
  
  if (pipe(fd) != 0)
  {
    perror("mikes:mapping_navig");
    mikes_log(ML_ERR, "creating pipe for mapping_navig");
    return;
  }
  
  register_tim571_callback(tim571_newdata_callback);
  register_t265_callback(t265_newpos_callback);
  register_hcsr04_callback(hcsr04_newdata_callback);
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
	need_new_hcsr04_data = value;
}
