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
#include "../passive/gridmap.h"
#include "../passive/x_sensor_fusion.h"
#include "mapping_navig.h"


#define ARC_DIST 2000 
#define MAX_RAY_DIST 5000
#define MAX_NUM_CROSSINGS 50


#define REPLAN_MAXIMUM_PERIOD 1000

#define HCSR04_FILTER_TOO_HIGH_CHANGE 30

#define DO_NOT_USE_PREF_HEADING -123

#define WAITING_ON_STEPS   5000

double min_arc_angle;

double tim_arc_angle; //angle to check in distance in target direction

static double angle_tolerance;

static uint16_t             dist_local_copy[TIM571_DATA_COUNT];
//static uint8_t              rssi_local_copy[TIM571_DATA_COUNT];
static uint16_t             gauss_dist[TIM571_DATA_COUNT];

//static uint8_t useless_buffer[10000];

static hcsr04_data_type		hcsr04_data_local_copy;

//static uint8_t useless_buffer2[10000];

static path_type path;
static int path_index;
static double path_heading;

static volatile double pose_x;
static volatile double pose_y;
static volatile double heading;

static double correction_heading;
static double correction_distance = 30;
static uint8_t override = 0;

static int gridmap_pose_x;
static int gridmap_pose_y;
static int initial_pose_x;
static int initial_pose_y;

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

long long last_replan_time_in_ms;
int request_replan = 0;

int first_navigation;

//crossing variables
static uint16_t crossings[MAX_NUM_CROSSINGS];
static uint8_t visited[MAX_NUM_CROSSINGS];
static double cross_pos[MAX_NUM_CROSSINGS][2];
static uint8_t cross_size = 0;
static uint8_t in_crossing = 0;

int32_t robot_trajectory[MAX_TRAJECTORY_SIZE][2];
uint32_t trajectory_size;


void gaussian_filter(uint16_t *gauss);
void find_arcs(uint16_t *dist, uint16_t arcs[][2], uint8_t *arcs_size);
double choose_best_dir(uint16_t arcs[][2], uint8_t arcs_size);
void process_crossing(uint16_t arcs[][2], uint8_t arcs_size);

double get_traveled_dist(){
	return sqrt(pose_x*pose_x+pose_y*pose_y);
}

void reset_correction_dist(){
	correction_distance = get_traveled_dist();
}

void correct_movement(){
	uint16_t us_left = hcsr04_data_local_copy[HCSR04_LEFT];
	uint16_t us_right = hcsr04_data_local_copy[HCSR04_RIGHT];
	if (us_left <= 0 || us_right <= 0){
		return;
	}
	int16_t us_diff = us_left - us_right;
	correction_heading = (us_diff / (us_left + us_right))*30 / 180.0 * M_PI;
	if (us_left > us_right) {
		correction_heading = -correction_heading;
	}
	//correction_heading = (us_right - us_left) / 180.0 * M_PI;
	mikes_log_double(ML_INFO, "correction_heading", correction_heading);
	reset_correction_dist();
}

void process_navigation(){
	sensor_fusion(hcsr04_data_local_copy, dist_local_copy);

	gaussian_filter(gauss_dist);
	
	uint16_t arcs[TIM571_DATA_COUNT][2];
	uint8_t arcs_size = 0;
	
	find_arcs(gauss_dist, arcs, &arcs_size);
	if (arcs_size > 1)
	{
		process_crossing(arcs, arcs_size);
	}
	else{
		in_crossing = 0;
	}
	target_heading = choose_best_dir(arcs, arcs_size);
	mikes_log_double(ML_INFO,"target_heading: ", target_heading);
	if (fabs(correction_distance-get_traveled_dist()) > 20){
		correct_movement();
	}
	target_heading +=correction_heading;
}

uint8_t front_sensors(int distance){
	if (((hcsr04_data_local_copy[HCSR04_TOP_LEFT] < distance) && (hcsr04_data_local_copy[HCSR04_TOP_LEFT] > 0)) || 
	((hcsr04_data_local_copy[HCSR04_TOP_RIGHT] < distance) && (hcsr04_data_local_copy[HCSR04_TOP_RIGHT] > 0)) || 
	((hcsr04_data_local_copy[HCSR04_MIDDLE_LEFT] < distance)  && (hcsr04_data_local_copy[HCSR04_MIDDLE_LEFT] > 0)) || 
	((hcsr04_data_local_copy[HCSR04_MIDDLE_RIGHT] < distance)  && (hcsr04_data_local_copy[HCSR04_MIDDLE_RIGHT] > 0 ))) {
		return 1;
	}
	return 0;
}

void log_hcsr04()
{
	for (int i = 0; i < 8; i++)
	  mikes_log_val2(ML_INFO, "US", i, hcsr04_data_local_copy[i]);
}

void override_movement(){
	reset_correction_dist();
	if ( ( hcsr04_data_local_copy[HCSR04_LEFT]  < 15 || hcsr04_data_local_copy[HCSR04_RIGHT] < 15 ) && front_sensors(35) ){
		set_motor_speeds(-12,-12);
		mikes_log(ML_INFO, "o1 motor -12, -12");
		override = 1;
		log_hcsr04();
	mikes_log(ML_INFO,"OVERRIDE  front and sides");
	}
	else if (hcsr04_data_local_copy[HCSR04_LEFT] < 15){
		correction_heading = M_PI_4;
		override = 0;
		log_hcsr04();
	mikes_log(ML_INFO,"OVERRIDE left side");
	}
	else if (hcsr04_data_local_copy[HCSR04_RIGHT] < 15){
		correction_heading = -M_PI_4;
		override = 0;
		log_hcsr04();
	mikes_log(ML_INFO,"OVERRIDE right side");
	}
	else if (front_sensors(35)){ // TODO >>>???
		set_motor_speeds(-12,-12);
		mikes_log(ML_INFO, "o1 motor -12, -12");
		override = 1;
		log_hcsr04();
	mikes_log(ML_INFO,"OVERRIDE front");
	}
}


void process_movement(){
		char logstr[100];
		sprintf(logstr, "process_movement: target %.3lf head %.3lf\n", target_heading / M_PI * 180, heading / M_PI * 180);
		mikes_log(ML_INFO, logstr);
	
	double heading_dif = angle_rad_difference(heading, target_heading);
	mikes_log_double(ML_INFO, "heading_diff:", heading_dif);
	//if (heading<target_heading+angle_tolerance && heading > target_heading - angle_tolerance)
	if (fabs(heading_dif) < angle_tolerance)
	{
		//set_motor_speeds(0,0);
		//perform map scan 			
		int turn_motor_speed = 12 - (int)(0.5 + 6 * (fabs(heading_dif)/angle_tolerance));
		
		if (heading_dif < 0){
   		    mikes_log_val2(ML_INFO, "turn motor speed:", turn_motor_speed, 10);
			set_motor_speeds(turn_motor_speed,12);
		}
		else{
			mikes_log_val2(ML_INFO, "turn motor speed:", 10, turn_motor_speed);
			set_motor_speeds(12,turn_motor_speed);
		}
	}
	else{
		if (angle_rad_difference(heading, target_heading) > 0){
		    mikes_log(ML_INFO, "extreme diff, rotate right 8 -8");
			set_motor_speeds(8, -8);
		}
		else{
		    mikes_log(ML_INFO, "extreme diff, rotate left -11 8");
			set_motor_speeds(-11, 8);
		}
	}
	need_new_pos = 1;
}

void process_crossing(uint16_t arcs[][2], uint8_t arcs_size)
{
	if (!pref_heading)
	{
		return;
	}
	if (arcs_size == 2)
	{
		return; //possible false cross when robot facing wall and seeing paths on both sides
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
		cross_pos[cross_size][0] = pose_x;
		cross_pos[cross_size][1] = pose_y;
		visited[cross_size] = 1;
		cross_size++;
		for (int i = 0; i < arcs_size; i++)
		{
			int middle_ray = arcs[i][0]+(arcs[i][1]-arcs[i][0])/2;
			crossings[cross_size] = tim571_ray2azimuth(middle_ray)/ 180.0 * M_PI + heading;
			cross_size++;
		}
	} 
	
	
}

uint8_t check_obstacles()
{
	static unsigned long no_steps_below = 0;
	static uint8_t we_are_stopped = 0;
	
	/*double angle = tim_arc_angle / M_PI * 180;
	int ray_start = tim571_azimuth2ray(angle / 2);
	int ray_end = tim571_azimuth2ray(- angle / 2);
	
	for (; ray_start < ray_end; ray_start++)
	{
		if (gauss_dist[ray_start] < target_distance){
			return 0;
		}
	}
	return 1;*/
	if (front_sensors(35) || hcsr04_data_local_copy[HCSR04_LEFT] < 15 || hcsr04_data_local_copy[HCSR04_RIGHT] < 15){
		return 1;
	}
	if (hcsr04_data_local_copy[HCSR04_DOWN_LEFT] > 25 || hcsr04_data_local_copy[HCSR04_DOWN_RIGHT] > 25){
		stop_now();
		no_steps_below = msec();
		we_are_stopped = 1;
		return 1;
	}
	else
	{
		if (we_are_stopped && (msec() - no_steps_below > WAITING_ON_STEPS))
		{
			 set_motor_speeds(8, 8);
			 we_are_stopped = 0;
		}
	}
		
	return 0;
}

uint8_t arc_valid(uint16_t *dist, uint16_t mid_ray){
	for (int i = 1; i < 4; i++){//check if robot can move towards arcs' mid
		double d = (target_distance / 4) * i;
		double angle = get_arc_angle_in_dist((WHEELS_DISTANCE + 150),d)*180/M_PI;
		int start_arc = (tim571_azimuth2ray(-angle/2));
		int end_arc = (tim571_azimuth2ray(angle/2));
		int offset = mid_ray - (start_arc + (int)(0.5+(end_arc - start_arc)/2) ) ;
		start_arc += offset;
		end_arc += offset;
		for (; start_arc < end_arc; start_arc++){
			if (dist[start_arc] < d * 10){
				return 0;
			}
		}
	}
	return 1;
}


void find_arcs(uint16_t *dist, uint16_t arcs[][2], uint8_t *arcs_size)
{
	uint8_t inside_arc = 0;
	uint16_t idx = 0;
	uint8_t skip;
	for(int i = 0; i < TIM571_DATA_COUNT; i++)
	{
		if (inside_arc)
		{
			if ((dist[i] < ARC_DIST) && (dist[i] > 10))
			{
				if (i - arcs[idx][0] >= min_arc_angle * 3 && arc_valid(dist, (uint16_t) (0.5 + ((i -1) - arcs[idx][0]) /2 )))
				{
					mikes_log_val(ML_INFO, "MIN ARC ANGLE", min_arc_angle);
					arcs[idx][1] = i - 1;
					mikes_log_val2(ML_INFO, "Mikes arc [start,end]", arcs[idx][0], i-1);
					idx++;
				}
				skip = 0;
				for (int j = 1; j < 4; j++){
					if (i + j < TIM571_DATA_COUNT && dist[j] > ARC_DIST){
						i+=j;
						skip = 1;
						break;
					}
				}
				if (skip){
					continue;
				}
				inside_arc = 0;
			}
		}
		else
		{
			if ((dist[i] >= ARC_DIST) || (dist[i] < 0))
			{
				arcs[idx][0] = i;
				inside_arc = 1;
			}
		}
	}
	*arcs_size = idx;
	
	
	char logtim[5000];
	for (int i = 0; i < 5000; i++) logtim[i] = ' ';
	for (int i = 0; i < TIM571_DATA_COUNT; i++)
	{
		int numchars = sprintf(logtim + i * 6, "%d", dist[i]);
		*(logtim + i * 6 + numchars) = ' ';
	}
	logtim[TIM571_DATA_COUNT * 6] = 0;
	mikes_log(ML_INFO, logtim);
	mikes_log_val(ML_INFO, "arcs: ", *arcs_size);
}

double choose_best_dir(uint16_t arcs[][2], uint8_t arcs_size)
{
	int angle;
	if (arcs_size == 0) // no arc found
	{
		/*
		if (!check_front_sensors())
		{
			return target_heading;
		}
		*/
		//turn around and go back( or reverse)
		angle = 45;
		first_navigation = 1;
		
		mikes_log_val(ML_INFO, "NO ARC FOUND", angle);

	}
	else{
		int best_arc = 0;
		if (pref_heading != DO_NOT_USE_PREF_HEADING && arcs_size>1)
		{
			double h_dif = 10;
			for (int i = 0; i < arcs_size; i++)
			{
				int middle_ray = arcs[i][0]+(arcs[i][1]-arcs[i][0])/2;
				angle = (int)(0.5 + tim571_ray2azimuth(middle_ray));
				if (fabs(pref_heading-(angle / 180.0 * M_PI + heading))< h_dif){
					best_arc = i;
					h_dif = fabs(pref_heading-(angle / 180.0 * M_PI + heading));
				}
			}
		}//TODO: choose best way in crossing
		
		//WATCH OUT FOR THE NEXT LINE
		// best_arc = 0;
		
		int middle_ray = arcs[best_arc][0]+(arcs[best_arc][1]-arcs[best_arc][0])/2;
		mikes_log_val(ML_INFO, "best_middle_ray", middle_ray);
		angle = (int)(0.5 + tim571_ray2azimuth(middle_ray));	
		mikes_log_val(ML_INFO, "best_middle_ray angle", angle);
		if (pref_heading == DO_NOT_USE_PREF_HEADING){
			pref_heading = angle / 180.0 * M_PI + heading;
		}
		//if (angle<0) angle+=360; 
		
		if (arcs_size != 1 && best_arc == arcs_size -1 && ((angle * M_PI/180.0 + heading)-pref_heading) < (65*M_PI/180.0)){//if angle < 60 - left way was probably chosen, we want right one
			best_arc = 0;	
			int middle_ray = arcs[best_arc][0]+(arcs[best_arc][1]-arcs[best_arc][0])/2;
			mikes_log_val(ML_INFO, "best_middle_ray correction", middle_ray);
			angle = (int)(0.5 + tim571_ray2azimuth(middle_ray));	
			mikes_log_val(ML_INFO, "best_middle_ray angle correction", angle);	
		}
	}
	return angle / 180.0 * M_PI + heading;	
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
			//mikes_log(ML_INFO, "mapping_navig gaussian_filter res_w = 0");
		}
	}
}


uint8_t check_front_sensors(){
	uint8_t min_us_range = 210;
	uint8_t min_bottom_range = 5;
	uint8_t max_bottom_range = 10;
	if (hcsr04_data_local_copy[HCSR04_MIDDLE_LEFT] < min_us_range || hcsr04_data_local_copy[HCSR04_TOP_LEFT] < min_us_range
	 || hcsr04_data_local_copy[HCSR04_DOWN_LEFT] < min_bottom_range || hcsr04_data_local_copy[HCSR04_DOWN_LEFT] > max_bottom_range)
	{
		return 1;
	}
	if (hcsr04_data_local_copy[HCSR04_MIDDLE_RIGHT] < min_us_range || hcsr04_data_local_copy[HCSR04_TOP_RIGHT] < min_us_range
	 || hcsr04_data_local_copy[HCSR04_DOWN_RIGHT] < min_bottom_range || hcsr04_data_local_copy[HCSR04_DOWN_RIGHT] > max_bottom_range)
	{
		return 2;
	}
	return 0;
}

void update_pose_on_gridmap(){
	gridmap_pose_x = (int)(0.5 + initial_pose_x + pose_x);
	gridmap_pose_y = (int)(0.5 + initial_pose_x - pose_y);
}

void add_pos_to_trajectory(){
	update_pose_on_gridmap();
	if (!trajectory_size || 
	     ((trajectory_size < MAX_TRAJECTORY_SIZE) && 
	      !((robot_trajectory[trajectory_size-1][0] == gridmap_pose_y) &&
	        (robot_trajectory[trajectory_size-1][1] == gridmap_pose_x)))){
		robot_trajectory[trajectory_size][0] = gridmap_pose_y;
		robot_trajectory[trajectory_size][1] = gridmap_pose_x;
		trajectory_size++;
		if (trajectory_size % 1000 == 0)
		  mikes_log_val(ML_INFO, "trajectory_size", trajectory_size);
	}
}

double dist_2pts(int x, int y, int a, int b){
	return sqrt((x - a) * (x - a) + (y - b) * (y - b));
}

uint8_t is_segment_closer(int new, int old){
	
	return 0;
	
}

void choose_best_dir_on_gridmap(){
	double **gridmap = (double **) malloc (mikes_config.gridmap_height * sizeof(double *));
	for (int i= 0; i< mikes_config.gridmap_height; i++){
		gridmap[i] = (double *) malloc (mikes_config.gridmap_width * sizeof(double));
	}
	get_gridmap_for_navigation(gridmap);
	update_pose_on_gridmap();
	uint16_t path8dir[8];
	uint8_t index[8] = {7,0,1,6,2,5,4,3};
	uint8_t c = 0;
	double h = heading;
	if (h < 0){
		h += M_2_PI;
	}
	if (h > M_2_PI){
		h -= M_2_PI;
	}
	int segment = (int) (0.5 + (h / M_PI_4));
	//int best_segment = 3;
	for (int i = - 1; i < 3; i++){
		for (int j = - 1; j < 3; j++){
			if (i == 0 && 0 == j){
				continue;
			}
			int dx = gridmap_pose_y + j;
			int dy = gridmap_pose_x + i;
			for (int a = 0; a < target_distance; a++){
				if (!cell_valid(dy,dx) || gridmap[dy][dx] > 0.35 || gridmap[dy][dx] < 0 || a == target_distance-1){
					path8dir[index[c]] = a;
					/*if (path8dir[best_segment] < path8dir[index[c]){
						if (path8dir[best_segment] < target_distance - 5){
							best_segment = index[c]
						}
						else{
							if (path8dir[index[c]] >= target_distance - 5 && (index[c] - 8 <  ) || ( ))
							
						}
					}
					
					*/
					c++;
					break;
				}
				dx+=j;
				dy+=i;
			}
		}	
	}
	if (segment == 0){
		if (path8dir[segment] >= target_distance-5){
			int res = path8dir[7] + path8dir[0] + path8dir[1];
			path_heading = heading + (path8dir[0]/res + (path8dir[7]/res - path8dir[1]/res)) * M_PI_4;
			return;
		}
	}
	if (segment == 7){
		if (path8dir[segment] >= target_distance-5){
			int res = path8dir[6] + path8dir[7] + path8dir[0];
			path_heading = heading + (path8dir[7]/res + (path8dir[6]/res - path8dir[0]/res)) * M_PI_4;
			return;
		}
	}
	if (segment != 0 || segment != 7){
		if (path8dir[segment] >= target_distance-5){
			int res = path8dir[segment - 1] + path8dir[segment] + path8dir[segment + 1];
			path_heading = heading + (path8dir[segment]/res + (path8dir[segment - 1]/res - path8dir[segment + 1]/res)) * M_PI_4;
			return;
		}
	}
	for (int i = 0; i < 8; i++){
		if (path8dir[i] >= target_distance-5){
			if (i == 0){
		if (path8dir[segment] >= target_distance-5){
			int res = path8dir[7] + path8dir[0] + path8dir[1];
			path_heading = heading + (path8dir[0]/res + (path8dir[7]/res - path8dir[1]/res)) * M_PI_4;
			return;
		}
	}
	if (i == 7){
		if (path8dir[segment] >= target_distance-5){
			int res = path8dir[6] + path8dir[7] + path8dir[0];
			path_heading = heading + (path8dir[7]/res + (path8dir[6]/res - path8dir[0]/res)) * M_PI_4;
			return;
		}
	}
	if (i != 0 || i != 7){
		if (path8dir[segment] >= target_distance-5){
			int res = path8dir[i-1] + path8dir[i] + path8dir[i+1];
			path_heading = heading + (path8dir[i]/res + (path8dir[i-1]/res - path8dir[i+1]/res)) * M_PI_4;
			return;
		}
	}
			
		}
	}
	
	
	path_heading = -5.0;
	return; 
}

void follow_path(){ // input gridmap.h path_type
	while (path_index < path.path_size){
		double dist_mikes_from_mappoint = dist_2pts(gridmap_pose_x, gridmap_pose_y, (*(path.path))[path_index][1], (*(path.path))[path_index][0]);
		if (dist_mikes_from_mappoint < 2.0){
			path_index++;
			continue;
		}
		break;
	}
	path_heading = get_robotangle2mappoint(gridmap_pose_x, gridmap_pose_y, heading, (*(path.path))[path_index][1], (*(path.path))[path_index][0]);
}


void tim571_newdata_callback(uint16_t *dist, uint8_t *rssi, tim571_status_data *status_data)
{
  if (need_new_data)
  {
    need_new_data = 0;
    memcpy(dist_local_copy, dist, sizeof(uint16_t) * TIM571_DATA_COUNT);
    //memcpy(rssi_local_copy, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
    long long time_in_ms = msec();
    if ((traveled_distance >= target_distance) || (time_in_ms - last_replan_time_in_ms > REPLAN_MAXIMUM_PERIOD))
    {
		tim_scan_pose_x = pose_x;
		tim_scan_pose_y = pose_y;
		last_replan_time_in_ms = time_in_ms;
		request_replan = 1;
	}
    alert_new_data(fd);
  }
}

void t265_newpos_callback(t265_pose_type *pose, double *new_heading)
{
  if (need_new_pos)
  {
	need_new_pos = 0;
	pose_x = pose->translation.x*100;
	pose_y = pose->translation.z*100; //?neg z value
	heading = - (*new_heading);  // we prefer clockwise positivity
	mikes_log_double(ML_INFO, "t265 x", pose_x);
	mikes_log_double(ML_INFO, "t265 y", pose_y);
	mikes_log_double(ML_INFO, "t265 nh", *new_heading);
	mikes_log_double(ML_INFO, "t265 h", heading);
	add_pos_to_trajectory();
  }
}

void hcsr04_newdata_callback(hcsr04_data_type hcsr04_data)
{
	static hcsr04_data_type last_hcsr04_data;
	static hcsr04_data_type last_last_hcsr04_data;
	
	for (int i = 0; i < 8; i++)
	{
		if (abs(last_hcsr04_data[i] - hcsr04_data[i]) > HCSR04_FILTER_TOO_HIGH_CHANGE)
		{
			if (abs(last_last_hcsr04_data[i] - hcsr04_data[i]) <= HCSR04_FILTER_TOO_HIGH_CHANGE)
				 hcsr04_data_local_copy[i] = hcsr04_data[i];
			else 
			     hcsr04_data_local_copy[i] = last_hcsr04_data[i];
		}
		else
			hcsr04_data_local_copy[i] = hcsr04_data[i];
	}
	
	memcpy(last_last_hcsr04_data, last_hcsr04_data, sizeof(uint16_t) * NUM_ULTRASONIC_SENSORS);		
	memcpy(last_hcsr04_data, hcsr04_data, sizeof(uint16_t) * NUM_ULTRASONIC_SENSORS);		
}

void *mapping_navig_thread(void *args)
{
  first_navigation = 1;
  in_crossing = 0;
  usleep(1500000L);

  long long time_in_ms = msec();
  last_replan_time_in_ms = time_in_ms;

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
		need_new_hcsr04_data = 1;
		//int chk_obst = check_obstacles();
		if (!first_navigation && override && !request_replan){
			if (fabs(correction_distance-get_traveled_dist()) > 20){
				override = 0 ;
			}
			request_replan = 1;
			mikes_log(ML_INFO, "thread overriding");
			//if (!request_replan){
				usleep(200000L);
				continue;
			//}
		}
		if (!first_navigation && check_obstacles() ){
			override_movement();
			if (!request_replan){
				continue;
			}
		}
		int chk_obst = 1;
		
		mikes_log_val(ML_INFO, "mapping_navig obs", chk_obst);
			
		if (first_navigation || (request_replan && chk_obst))
		{ //make a new scan
			mikes_log(ML_INFO, "mikes:mapping_navig newscan");
			first_navigation = 0;
			request_replan = 0;
			

			start_scanning();
			process_navigation();	
			/*choose_best_dir_on_gridmap();
			mikes_log_double(ML_INFO, "mapping_navig path_heading", path_heading);

			if (path_heading < -4){
				process_navigation();
			}
			else{
				target_heading = path_heading;
			}*/
		}
		else			//continue target heading
		{
			process_movement();
		}
		//x_gridmap_pose_changed();
		need_new_data = 1;
		need_new_pos = 1;
		need_new_hcsr04_data = 1;
	}
  }

  mikes_log(ML_INFO, "mapping_navig quits.");
  threads_running_add(-1);
  return 0;
}

void init_mapping_navig(){

  /*double suma;
  for (int i = 0; i < 10000; i++) useless_buffer[i] = useless_buffer2[10000 - i - 1] = i;
  for (int i = 0; i < 10000; i++) suma += useless_buffer[i] + useless_buffer2[i];
  printf("%e", suma);
  */
	//robot_trajectory = (uint16_t *) malloc (mikes_config.gridmap_height*10 * sizeof(uint16_t *));
  initial_pose_x = mikes_config.gridmap_width/2*10;
  initial_pose_y = mikes_config.gridmap_height/2*10;
  need_new_data = 1;
  need_new_pos = 1;
  need_new_hcsr04_data = 1;
  target_heading = 0;
  correction_heading = 0;
  pref_heading = DO_NOT_USE_PREF_HEADING;
  angle_tolerance = 30.0 / 180.0 * M_PI;
  tim_arc_angle = get_arc_angle_in_dist((WHEELS_DISTANCE + 150),target_distance);
  min_arc_angle = get_arc_angle_in_dist((WHEELS_DISTANCE + 150), ARC_DIST) * 180/ M_PI;
  mikes_log_double(ML_INFO, "TIM ARC ANGLE", min_arc_angle* 180/ M_PI);
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
