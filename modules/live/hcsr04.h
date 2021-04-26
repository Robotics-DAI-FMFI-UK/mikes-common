#ifndef __HCSR04_H__
#define __HCSR04_H__

#include <stdint.h>

#define NUM_ULTRASONIC_SENSORS 8

typedef uint16_t hcsr04_data_type[NUM_ULTRASONIC_SENSORS];

#define HCSR04_LEFT        0
#define HCSR04_TOP_LEFT    1
#define HCSR04_TOP_RIGHT   2
#define HCSR04_RIGHT       3
#define HCSR04_MIDDLE_LEFT 4
#define HCSR04_MIDDLE_RIGHT 5
#define HCSR04_DOWN_LEFT   6
#define HCSR04_DOWN_RIGHT  7

// distance(TOP_LEFT, TOP_RIGHT) = 19 cm
// forward_distance(TIM, TOP_LEFT,TOP_RIGHT) = 5 cm
// distance(LEFT, RIGHT) = 31 cm
// backward_distance(TIM, LEFT,RIGHT) = 10 cm
// distance(MIDDLE_LEFT, MIDDLE_RIGHT) = 16 cm

// these functions return the positions of the sensors
// relative to the laser range sensor center and forward heading
int hcsr04_get_sensor_posx(int sensor_index);
int hcsr04_get_sensor_posy(int sensor_index);
double hcsr04_get_sensor_heading(int sensor_index);


typedef void (*hcsr04_receive_data_callback)(hcsr04_data_type);

void get_hcsr04_data(hcsr04_data_type buffer);

void init_hcsr04();

void shutdown_hcsr04();

// register for getting fresh data after received from hcsr04 (copy quick!)
void register_hcsr04_callback(hcsr04_receive_data_callback callback);

// remove previously registered callback
void unregister_hcsr04_callback(hcsr04_receive_data_callback callback);

#endif
