#include <string.h>
#include <stdio.h>

#include "x_sensor_fusion.h"
#include "../live/gui.h"
#include "mikes_logs.h"
#include "core/config_mikes.h"
#include "../../bites/mikes.h"

#include "../live/tim571.h"

#include "../live/hcsr04.h"

#define ENLARGE_CENTER 50

#define RAY_USUAL_TYPE 1
#define RAY_AZIMUTH_TYPE 2
#define RAY_ZERO_TYPE 3

#define BASIC_LINE 1
#define FILTERED_LINE 2

static int win;
static uint16_t             dist_local_copy[TIM571_DATA_COUNT];
static uint8_t              rssi_local_copy[TIM571_DATA_COUNT];
static tim571_status_data   status_local_copy;


static volatile uint8_t  need_new_data;
static volatile uint8_t  need_new_hcsr04_data;

static hcsr04_data_type		hcsr04_data_local_copy;

static int range;
static double scale_factor;


double get_arc_width_in_dist(double angle, double dist){
	return sqrt(2*dist*dist- 2*dist*dist*cos(angle));
}

double get_height_of_arc_width(double angle, double dist){
	return cos(angle/2)*dist;
}

void sensor_fusion() // TIM571 + ULTRASONIC
{
	for (int i = 0; i < NUM_ULTRASONIC_SENSORS-2; i++)
	{
		if (hcsr04_data_local_copy[i] > 450 || hcsr04_data_local_copy[i] <= 0){
			continue;
		}
		int x = hcsr04_get_sensor_posx(i);
		int y = hcsr04_get_sensor_posy(i);
		int head = hcsr04_get_sensor_heading(i);
		double w = get_arc_width_in_dist(HCSR04_SCAN_ANGLE/180.0 * M_PI, hcsr04_data_local_copy[i]);
		double h = get_height_of_arc_width(HCSR04_SCAN_ANGLE/180.0 * M_PI, hcsr04_data_local_copy[i]);
		
		double w1_x = x - w/2; 
		double w1_y = y + h;
		double w1_dist = sqrt(w1_x * w1_x + w1_y * w1_y);
		
		double w2_x = x + w/2;
		double w2_y = y + h;
		double w2_dist = sqrt(w2_x * w2_x + w2_y * w2_y);
		
		double tim_angle = acos((w1_dist * w1_dist + w2_dist * w2_dist - w * w) / (2 * w1_dist * w2_dist));
		//double theta = acos((( x + w/2)*( x + w/2) + w2_dst * w2_dist - (y + h) * (y + h)) / (2 * w2_dist * ( x + w/2)));
		double theta = acos(((y + h) * (y + h) + w1_dist * w1_dist - ( x - w/2) * ( x - w/2)) / (2 * w1_dist * ( y + h))) + head;
		
		int tim_ray_start = tim571_azimuth2ray((int)0.5+theta);
		int tim_ray_end = tim571_azimuth2ray((int)0.5+theta+tim_angle);
		double hop = w / (tim_ray_end - tim_ray_start);
		
		int count = 0;
		for (int j = tim_ray_start; j < tim_ray_end; j++){
			dist_local_copy[j] = w1_dist + hop * count;
			count++;
		}
	}	
}

void tim571_draw_ray(cairo_t *w, int i, uint16_t d, uint8_t q, int ray_type)
{
   int x = 0, y=0, center_x=0, center_y=0;
   double alpha = M_PI * tim571_ray2azimuth(i) / 180.0;
   //printf("%5.3lf (%5.3lf): %d (%d)\n", tim571_ray2azimuth(i), alpha, d, q);

   if (ray_type == RAY_USUAL_TYPE)
   {
      x = (int)((ENLARGE_CENTER + d) / scale_factor * X_SENSOR_FUSION_WIDTH * 0.45 * sin(alpha) + X_SENSOR_FUSION_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + d) / scale_factor * X_SENSOR_FUSION_HEIGHT * 0.45 * cos(alpha) + X_SENSOR_FUSION_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_SENSOR_FUSION_WIDTH * 0.45 * sin(alpha) + X_SENSOR_FUSION_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_SENSOR_FUSION_HEIGHT * 0.45 * cos(alpha) + X_SENSOR_FUSION_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.7 * (255 - q) / 255, 0.7 * (255 - q) / 255, 0.3 + 0.7 * q / 255.0);
   } else if (ray_type == RAY_AZIMUTH_TYPE)
   {
      x = (int)((ENLARGE_CENTER + range) / scale_factor * X_SENSOR_FUSION_WIDTH * 0.45 * sin(M_PI * i / 64.0 / 180.0) + X_SENSOR_FUSION_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + range) / scale_factor * X_SENSOR_FUSION_HEIGHT * 0.45 * cos(M_PI * i / 64.0 / 180.0) + X_SENSOR_FUSION_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_SENSOR_FUSION_WIDTH * 0.45 * sin(M_PI * i / 64.0 / 180.0) + X_SENSOR_FUSION_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_SENSOR_FUSION_WIDTH * 0.45 * cos(M_PI * i / 64.0 / 180.0) + X_SENSOR_FUSION_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.8, 0.8, 0.3);
   } else if (ray_type == RAY_ZERO_TYPE)
   {
      x = (int)((ENLARGE_CENTER + range) / scale_factor * X_SENSOR_FUSION_WIDTH * 0.45 * sin(alpha) + X_SENSOR_FUSION_WIDTH / 2);
      y = (int)((ENLARGE_CENTER + range) / scale_factor * X_SENSOR_FUSION_HEIGHT * 0.45 * cos(alpha) + X_SENSOR_FUSION_HEIGHT / 2);
      center_x = (int)(ENLARGE_CENTER / scale_factor * X_SENSOR_FUSION_WIDTH * 0.45 * sin(alpha) + X_SENSOR_FUSION_WIDTH / 2);
      center_y = (int)(ENLARGE_CENTER / scale_factor * X_SENSOR_FUSION_HEIGHT * 0.45 * cos(alpha) + X_SENSOR_FUSION_HEIGHT / 2);
      cairo_set_source_rgb(w, 0.8, 0.8, 0.8);
   }

   cairo_move_to(w, x, X_SENSOR_FUSION_HEIGHT - y);
   cairo_line_to(w, center_x, X_SENSOR_FUSION_HEIGHT - center_y);
   cairo_stroke(w);
   if (ray_type == RAY_USUAL_TYPE)
      cairo_set_source_rgb(w, q / 255.0, 0.7 * (255 - q) / 255, 0.3);
   else if (ray_type == RAY_AZIMUTH_TYPE)
      cairo_set_source_rgb(w, 0.8, 0.8, 0.3);
   else if (ray_type == RAY_ZERO_TYPE)
      cairo_set_source_rgb(w, 0.8, 0.8, 0.8);
   else cairo_set_source_rgb(w, 1, 0.3, 0.3);

   cairo_arc(w, x, X_SENSOR_FUSION_HEIGHT - y, 2, 0, 2 * M_PI);
   cairo_stroke(w);
}

void x_sensor_fusion_paint(cairo_t *w)
{
  cairo_push_group(w);
	sensor_fusion();
  cairo_set_source_rgb(w, 1, 1, 1);
  cairo_paint(w);
  cairo_set_line_width(w, 1);

  for (int i = 0; i < TIM571_DATA_COUNT; i++)
  {
    uint16_t d = dist_local_copy[i];
    uint8_t  r = rssi_local_copy[i];

    if (d > range) d = range;
    tim571_draw_ray(w, i, d, r, (d == 0) ? RAY_ZERO_TYPE : RAY_USUAL_TYPE);
  }

  cairo_pop_group_to_source(w);
  cairo_paint(w);
  need_new_data = 1;
  need_new_hcsr04_data = 1;
}

void x_sensor_fusion_laser_update(uint16_t *dist, uint8_t *rssi, tim571_status_data *status_data)
{
	if (need_new_data){
		need_new_data = 0;
		memcpy(dist_local_copy, dist, sizeof(uint16_t) * TIM571_DATA_COUNT);
		memcpy(rssi_local_copy, rssi, sizeof(uint8_t) * TIM571_DATA_COUNT);
	}
}

void x_sensor_fusion_key_listener(int win, int key)
{
    if (key == GUI_ESC_KEY) program_runs = 0;
}

void x_sensor_fusion_hcsr04_update(hcsr04_data_type hcsr04_data)
{
  if (need_new_hcsr04_data)
  {
		need_new_hcsr04_data = 0;
		memcpy(hcsr04_data_local_copy, hcsr04_data, sizeof(uint16_t) * NUM_ULTRASONIC_SENSORS);
  }
}

void init_x_sensor_fusion(int max_range_in_mm, int window_update_period_in_ms)
{
   if (!mikes_config.with_gui) return;
   if (!mikes_config.use_tim571)
   {
        mikes_log(ML_INFO, "tim571-sensor_fusion gui supressed by config.");
        return;
   }

   range = max_range_in_mm;
   scale_factor = range;
	need_new_data = 1;
	need_new_hcsr04_data = 1;
   win = gui_open_window(x_sensor_fusion_paint, X_SENSOR_FUSION_WIDTH, X_SENSOR_FUSION_HEIGHT, window_update_period_in_ms);
   gui_set_window_title(win, "SENSOR FUSION");
   register_tim571_callback(x_sensor_fusion_laser_update);
   register_hcsr04_callback(x_sensor_fusion_hcsr04_update);
   get_tim571_status_data(&status_local_copy);
   gui_add_key_listener("showing", x_sensor_fusion_key_listener);
}

void shutdown_x_sensor_fusion()
{
   if (!mikes_config.with_gui) return;
   if (!mikes_config.use_tim571)
   gui_close_window(win);
}
