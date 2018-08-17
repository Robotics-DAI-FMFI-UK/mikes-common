#ifndef _UST10LX_H
#define _UST10LX_H

#include <pthread.h>
#include <math.h>
#include <stdint.h>

/* ray integer calculations are based on the fact that UST-10LX takes exactly 4 rays per degree */
#define UST10LX_DATA_COUNT 1081
#define UST10LX_TOTAL_ANGLE (3 * M_PI / 2)
#define UST10LX_TOTAL_ANGLE_DEG 270
#define UST10LX_SIZE_OF_ONE_STEP (UST10LX_TOTAL_ANGLE / (UST10LX_DATA_COUNT - 1))
#define UST10LX_SIZE_OF_ONE_DEG ((UST10LX_DATA_COUNT - 1) / UST10LX_TOTAL_ANGLE_DEG)
#define UST10LX_PORT 10940
#define UST10LX_ADDR "169.254.0.10"
#define UST10LX_MAX_DISTANCE 8000

typedef void (*ust10lx_receive_data_callback)(uint16_t *);

void init_ust10lx();
void get_ust10lx_data(uint16_t* buffer);

// register for getting fresh data after received from sensor (copy quick!)
void register_ust10lx_callback(ust10lx_receive_data_callback callback);    

// remove previously registered callback
void unregister_ust10lx_callback(ust10lx_receive_data_callback callback);  

/* ray: 0..UST10LX_DATA_COUNT - 1, returns: 0-360 */
int ust10lx_ray2azimuth(uint16_t ray);

/* alpha: -180..360, returns: ray 0..max, max = UST10LX_DATA_COUNT - 1 (out of range clips to 0 or to max) */
uint16_t ust10lx_azimuth2ray(int alpha);

#endif

