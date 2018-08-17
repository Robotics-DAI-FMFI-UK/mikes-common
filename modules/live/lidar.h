#ifndef _LIDAR_H
#define _LIDAR_H

#include <stdint.h>
#include <pthread.h>

#define MAX_LIDAR_DATA_COUNT 720

typedef struct lidarstruct {
  uint16_t count;
  uint8_t quality[MAX_LIDAR_DATA_COUNT];
  uint16_t distance[MAX_LIDAR_DATA_COUNT];   // [mm]
  uint16_t angle[MAX_LIDAR_DATA_COUNT];      // [deg * 64]
} lidar_data_type;

typedef void (*lidar_receive_data_callback)(lidar_data_type *);

void init_lidar();
void get_lidar_data(lidar_data_type *buffer);

// register for getting fresh data after received from sensor (copy quick!)
void register_lidar_callback(lidar_receive_data_callback callback);    

// remove previously registered callback
void unregister_lidar_callback(lidar_receive_data_callback callback);  

#endif
