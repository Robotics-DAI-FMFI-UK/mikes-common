#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include <rplidar.h>

extern "C" {
    
#include "../../bites/mikes.h"
#include "lidar.h"
#include "../passive/mikes_logs.h"
#include "core/config_mikes.h"

}

#define LIDAR_PORT "/dev/lidar"
#define LIDAR_BAUD_RATE 115200

#define MAX_LIDAR_CALLBACKS 20

using namespace rp::standalone::rplidar;

static pthread_mutex_t lidar_lock;
static rplidar_response_measurement_node_t *lidar_data;
static size_t lidar_data_count;

static RPlidarDriver *drv;

static rplidar_response_measurement_node_t *local_data;
static size_t local_data_count;

static lidar_receive_data_callback callbacks[MAX_LIDAR_CALLBACKS];
static int callbacks_count;

static lidar_data_type lidar_data_for_callbacks;

static int online;

int connect_lidar()
{
    // create the driver instance
    drv = RPlidarDriver::CreateDriver(); 
    sleep(1);

    if (!drv) 
    {
        mikes_log(ML_ERR, "rplidar: insufficent memory, exit");
        return 0;
    }

    rplidar_response_device_health_t healthinfo;
    rplidar_response_device_info_t devinfo;
    // try to connect
    if (IS_FAIL(drv->connect(LIDAR_PORT, LIDAR_BAUD_RATE))) 
    {
        mikes_log(ML_ERR, "rplidar: error, cannot bind to the specified serial port");
        return 0;
    }

    // retrieving the device info
    ////////////////////////////////////////
    u_result op_result = drv->getDeviceInfo(devinfo);

    if (IS_FAIL(op_result)) 
    {
        if (op_result == RESULT_OPERATION_TIMEOUT) 
            // you can check the detailed failure reason
            mikes_log(ML_ERR, "rplidar: error, operation time out");
        else 
            mikes_log_val(ML_ERR, "rplidar: error, unexpected error ", op_result);
            // other unexpected result
        return 0;
    }

    // check the device health
    ////////////////////////////////////////
    op_result = drv->getHealth(healthinfo);
    if (IS_OK(op_result)) 
     // the operation has succeeded
        switch (healthinfo.status) 
        {
            case RPLIDAR_STATUS_OK:
                mikes_log(ML_INFO, "RPLidar health status : OK.");
                break;
            case RPLIDAR_STATUS_WARNING:
                mikes_log(ML_INFO, "RPLidar health status : Warning.");
                mikes_log_val(ML_INFO, "lidar errorcode: ", healthinfo.error_code);
                break;
            case RPLIDAR_STATUS_ERROR:
                mikes_log(ML_ERR, "RPLidar health status : Error.");
                mikes_log_val(ML_ERR, "lidar errorcode: ", healthinfo.error_code);
                return 0;
        }
    else 
    {
        mikes_log_val(ML_ERR, "Error, cannot retrieve the lidar health code: ", op_result);
        return 0;
    }


    if (healthinfo.status == RPLIDAR_STATUS_ERROR) 
    {
        mikes_log(ML_ERR, "Error, rplidar internal error detected. Please reboot the device to retry.");
        // enable the following code if you want rplidar to be reboot by software
        // drv->reset();
        return 0;
    }

    drv->startMotor();
    sleep(2);
    mikes_log(ML_INFO, "lidar connected");
    return 1;
}

void get_lidar_data_without_lock(lidar_data_type *buffer)
{
    for (size_t i = 0; i < lidar_data_count; ++i) 
    {
        buffer->quality[i] = lidar_data[i].sync_quality >> 2;   // syncbit:1;syncbit_inverse:1;quality:6;
        buffer->angle[i] = lidar_data[i].angle_q6_checkbit >> 1; // check_bit:1;angle_q6:15;
        buffer->distance[i] = lidar_data[i].distance_q2;
    }
    buffer->count = lidar_data_count;
}

void get_lidar_data(lidar_data_type *buffer)
{
    if (!online) return;

    pthread_mutex_lock(&lidar_lock);
    get_lidar_data_without_lock(buffer);
    pthread_mutex_unlock(&lidar_lock);
}

void *lidar_thread(void *args)
{
    while (program_runs)
    {
        int erri = 0;
        while (erri < 30) 
        {
            if (IS_FAIL(drv->startScan(false, true))) 
            {
                mikes_log_val(ML_ERR, "rplidar error, cannot start the scan operation. Trying again.", erri);
                ++erri;
                usleep(50000);
            } else break;
        
        }
        if (erri == 30) 
        { 
            mikes_log(ML_ERR, "rplidar error, cannot start the scan operation. End.");
            break;
        }

        u_result ans;    
        local_data_count = MAX_LIDAR_DATA_COUNT;

        // fetech extactly one 0-360 degrees' scan
        ans = drv->grabScanData(local_data, local_data_count);
        if (IS_OK(ans) || ans == RESULT_OPERATION_TIMEOUT) 
            drv->ascendScanData(local_data, local_data_count);
        else 
            mikes_log_val(ML_ERR, "lidar error code: ", ans);
        
        pthread_mutex_lock(&lidar_lock);
        memcpy(lidar_data, local_data, local_data_count * sizeof(rplidar_response_measurement_node_t));
        lidar_data_count = local_data_count;
        pthread_mutex_unlock(&lidar_lock);

        if (callbacks_count > 0)
        {
            get_lidar_data_without_lock(&lidar_data_for_callbacks);
            for (int i = 0; i < callbacks_count; i++)
                callbacks[i](&lidar_data_for_callbacks);
        }
        usleep(45000);
    }

    drv->stop();
    drv->stopMotor();

    RPlidarDriver::DisposeDriver(drv);

    free(lidar_data);
    free(local_data);
    
    mikes_log(ML_INFO, "lidar quits.");
    threads_running_add(-1);
    return 0;
}

void init_lidar()
{
    if (!mikes_config.use_rplidar)
    {
        mikes_log(ML_INFO, "rplidar supressed by config.");
        online = 0;
        return;
    }
    online = 1;

    pthread_t t;
    lidar_data = (rplidar_response_measurement_node_t *) malloc(sizeof(rplidar_response_measurement_node_t) * MAX_LIDAR_DATA_COUNT);
    local_data = (rplidar_response_measurement_node_t *) malloc(sizeof(rplidar_response_measurement_node_t) * MAX_LIDAR_DATA_COUNT);
    if ((lidar_data == 0) || (local_data == 0) )
    {
      perror("mikes:lidar");
      mikes_log(ML_ERR, "rplidar: insufficient memory");
      exit(1);
    }
    pthread_mutex_init(&lidar_lock, 0);
    if (!connect_lidar())
    {
      mikes_log(ML_ERR, "connect lidar returned 0, exiting");
      exit(1);
    }
    if (pthread_create(&t, 0, lidar_thread, 0) != 0)
    {
      perror("mikes:lidar");
      mikes_log(ML_ERR, "creating thread for lidar");
    }
    else threads_running_add(1);
}

// register for getting fresh data after received from sensor (copy quick!)
void register_lidar_callback(lidar_receive_data_callback callback)
{
  if (!online) return;

  if (callbacks_count == MAX_LIDAR_CALLBACKS)
  {
     mikes_log(ML_ERR, "too many lidar callbacks");
     return;
  }
  callbacks[callbacks_count] = callback;
  callbacks_count++;
}

// remove previously registered callback
void unregister_lidar_callback(lidar_receive_data_callback callback)
{
  if (!online) return;

  for (int i = 0; i < callbacks_count; i++)
    if (callbacks[i] == callback)
    {
       callbacks[i] = callbacks[callbacks_count - 1];
       callbacks_count--;
    }
}

