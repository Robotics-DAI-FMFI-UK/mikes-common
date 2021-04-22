#ifndef __HCSR04_H__
#define __HCSR04_H__

#include <stdint.h>

#define NUM_ULTRASONIC_SENSORS 8

typedef uint16_t hcsr04_data_type[NUM_ULTRASONIC_SENSORS];

typedef void (*hcsr04_receive_data_callback)(hcsr04_data_type);

void get_hcsr04_data(hcsr04_data_type buffer);

void init_hcsr04();

void shutdown_hcsr04();

// register for getting fresh data after received from hcsr04 (copy quick!)
void register_hcsr04_callback(hcsr04_receive_data_callback callback);

// remove previously registered callback
void unregister_hcsr04_callback(hcsr04_receive_data_callback callback);

#endif
