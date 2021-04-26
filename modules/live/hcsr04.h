#ifndef __HCSR04_H__
#define __HCSR04_H__

#include <stdint.h>

#define NUM_ULTRASONIC_SENSORS 8

typedef uint16_t hcsr04_data_type[NUM_ULTRASONIC_SENSORS];

typedef struct hcsr04_str{
		uint16_t LEFT;
		uint16_t TOP_LEFT;
		uint16_t TOP_RIGHT;
		uint16_t RIGHT;
		uint16_t MIDDLE_LEFT;
		uint16_t MIDDLE_RIGHT;
		uint16_t DOWN_LEFT;
		uint16_t DOWN_RIGHT;
	} hcsr04_data_type1;

typedef void (*hcsr04_receive_data_callback)(hcsr04_data_type);

void get_hcsr04_data(hcsr04_data_type buffer);

void init_hcsr04();

void shutdown_hcsr04();

// register for getting fresh data after received from hcsr04 (copy quick!)
void register_hcsr04_callback(hcsr04_receive_data_callback callback);

// remove previously registered callback
void unregister_hcsr04_callback(hcsr04_receive_data_callback callback);

#endif
