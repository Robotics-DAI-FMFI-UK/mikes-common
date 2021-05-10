#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

#include "hcsr04.h"
#include "../../bites/mikes.h"
#include "../passive/mikes_logs.h"
#include "../../bites/util.h"


#define MAX_HCSR04_CALLBACKS 20

#define US_FAR_ENOUGH 100
#define US_INIFINITELY_FAR 600

static volatile hcsr04_receive_data_callback callbacks[MAX_HCSR04_CALLBACKS];
static volatile int callbacks_count;

static int fdR[2];
static int fdW[2];
static pid_t plink_child;
static volatile unsigned char hcsr04_initialized;

static pthread_mutex_t hcsr04_module_lock;

static hcsr04_data_type local_hcsr04_data;


// distance(TOP_LEFT, TOP_RIGHT) = 19 cm
// forward_distance(TIM, TOP_LEFT,TOP_RIGHT) = 5 cm
// distance(LEFT, RIGHT) = 31 cm
// backward_distance(TIM, LEFT,RIGHT) = 10 cm
// distance(MIDDLE_LEFT, MIDDLE_RIGHT) = 16 cm

// these functions return the positions of the sensors
// relative to the laser range sensor center and forward heading

int hcsr04_get_sensor_posx(int sensor_index)
{
	if (sensor_index == HCSR04_TOP_LEFT)
	{
		return -10;
	}
	if (sensor_index == HCSR04_TOP_RIGHT)
	{
		return 10;
	}
	if (sensor_index == HCSR04_MIDDLE_LEFT)
	{
		return -8;
	}
	if (sensor_index == HCSR04_MIDDLE_RIGHT)
	{
		return 8;
	}
	if (sensor_index == HCSR04_LEFT)
	{
		return -15;
	}
	if (sensor_index == HCSR04_RIGHT)
	{
		return 15;
	}
	return 0;
}

int hcsr04_get_sensor_posy(int sensor_index)
{
	if (sensor_index == HCSR04_TOP_LEFT || sensor_index == HCSR04_TOP_RIGHT || sensor_index == HCSR04_MIDDLE_LEFT || sensor_index == HCSR04_MIDDLE_RIGHT)
	{
		return 5;
	}
	if (sensor_index == HCSR04_LEFT || sensor_index == HCSR04_RIGHT)
	{
		return -10;
	}
	return 0;
}

double hcsr04_get_sensor_heading(int sensor_index)
{
	if (sensor_index == HCSR04_LEFT)
	{
		return - (M_PI_2);
	}
	if (sensor_index == HCSR04_RIGHT)
	{
		return M_PI_2;
	}
	return 0.0;
}


void connect_hcsr04()
{
    if (pipe(fdR) < 0)
    {
        mikes_log(ML_ERR, "hcsr04: pipe2()");
        hcsr04_initialized = 0;
        return;
    }
    
    if (pipe(fdW) < 0)
    {
        mikes_log(ML_ERR, "hcsr04: pipe2()");
        hcsr04_initialized = 0;
        return;
    }

    if ((plink_child = fork()) == 0)
    {
        close(0);
        close(1);
        dup2(fdR[0], 0);
        dup2(fdW[1], 1);
        close(fdR[0]);
        close(fdR[1]);
        close(fdW[0]);
        close(fdW[1]);

        if (execl("/usr/bin/plink", "/usr/bin/plink", "/dev/ultrasonic",
                  "-serial", "-sercfg", "115200,N,n,8,1", NULL) < 0)
        {
            mikes_log(ML_ERR, "hcsr04: child execl()");
            hcsr04_initialized = 0;
            return;
        }
    }

    if (plink_child < 0)
    {
        mikes_log(ML_ERR, "hcsr04: child execl()");
        hcsr04_initialized = 0;
        return;
    }

    close(fdR[0]);
    close(fdW[1]);
    if (fcntl( fdW[0], F_SETFL, fcntl(fdW[0], F_GETFL) | O_NONBLOCK) < 0)
    {
        mikes_log(ML_ERR, "hcsr04: setting nonblock on read pipe end");
        hcsr04_initialized = 0;
    }

    mikes_log(ML_INFO, "hcsr04 module connected");
    sleep(2); // arduino power up 
    hcsr04_initialized = 1;
}

void filter_hcsr04()
{
    if (local_hcsr04_data[0] > US_FAR_ENOUGH) local_hcsr04_data[0] = US_INIFINITELY_FAR;
    if (local_hcsr04_data[1] > US_FAR_ENOUGH) local_hcsr04_data[1] = US_INIFINITELY_FAR;
    if (local_hcsr04_data[2] > US_FAR_ENOUGH) local_hcsr04_data[2] = US_INIFINITELY_FAR;
    if (local_hcsr04_data[3] > US_FAR_ENOUGH) local_hcsr04_data[3] = US_INIFINITELY_FAR;
    if (local_hcsr04_data[4] > US_FAR_ENOUGH) local_hcsr04_data[4] = US_INIFINITELY_FAR;
    if (local_hcsr04_data[5] > US_FAR_ENOUGH) local_hcsr04_data[5] = US_INIFINITELY_FAR;
    
}

void read_hcsr04_packet()
{
    unsigned char ch;
    int numRead;

    do {
		ch = 0;
        if ((numRead = read(fdW[0], &ch, 1)) < 0)
        {
            if (errno != EAGAIN)
            {
                if (!hcsr04_initialized) return;
                perror("read()");
                mikes_log(ML_ERR, "read from base err");
                exit(-1);
            }
            else usleep(2000);  
        }
                
    } while (program_runs && (ch != 'u'));

    unsigned char more_packets_in_queue = 0;
    char line[1024];
    do {
        int lnptr = 0;
        do {
          if ((numRead = read(fdW[0], line + lnptr, 1)) < 0)
          {
              if (errno != EAGAIN)
              {
                  if (!hcsr04_initialized) return;
                  perror("read()");
                  mikes_log(ML_ERR, "read from hcsr04 err2");
                  exit(-1);
              }
              else { usleep(2000); continue; }
          }
          lnptr += numRead;
          if (lnptr > 1023) break;
          if (lnptr == 0) continue;
        } while (program_runs && (line[lnptr - 1] != '\n'));

        if (lnptr > 0) line[lnptr - 1] = 0;

        more_packets_in_queue = 0;
        while (program_runs)
        {
			      ch = 0;
            if (read(fdW[0], &ch, 1) < 0)
            {
                if (errno == EAGAIN) break;
                else
                {
                    if (!hcsr04_initialized) return;
                    perror("read()");
                    exit(-1);
                }
            }
            if (ch == 'u')
            {
                more_packets_in_queue = 1;
                break;
            }
        }       

    } while (program_runs && more_packets_in_queue);

    pthread_mutex_lock(&hcsr04_module_lock);
    
    mikes_log(ML_INFO, "ARUS");
    mikes_log(ML_INFO, line);
      
    sscanf(line + 2, "%hu%hu%hu%hu%hu%hu%hu%hu",
                                  local_hcsr04_data, 
                                  local_hcsr04_data + 1, 
                                  local_hcsr04_data + 2, 
                                  local_hcsr04_data + 3, 
                                  local_hcsr04_data + 4, 
                                  local_hcsr04_data + 5, 
                                  local_hcsr04_data + 6, 
                                  local_hcsr04_data + 7);
    filter_hcsr04();

    //printf("%s\n", line);
    pthread_mutex_unlock(&hcsr04_module_lock);
    //mikes_log(ML_INFO, line);

    for (int i = 0; i < callbacks_count; i++)
      callbacks[i](local_hcsr04_data);
}

void *hcsr04_module_thread(void *args)
{
    while (program_runs && hcsr04_initialized)
    {
        read_hcsr04_packet();        
        usleep(10000);
    }

    mikes_log(ML_INFO, "hcsr04 quits.");    
    usleep(100000);
    kill(plink_child, SIGTERM);
    threads_running_add(-1);
    return 0;
}

void init_hcsr04()
{
    hcsr04_initialized = 0;
    connect_hcsr04();
    pthread_t t;
    if (!hcsr04_initialized) return;
    pthread_mutex_init(&hcsr04_module_lock, 0);
    if (pthread_create(&t, 0, hcsr04_module_thread, 0) != 0)
    {
      perror("mikes:hcsr04");
      mikes_log(ML_ERR, "creating thread for hcsr04 module");
      hcsr04_initialized = 0;
    }
    else threads_running_add(1);
}

void get_hcsr04_data(hcsr04_data_type buffer)
{
    if (!hcsr04_initialized) return;
    pthread_mutex_lock(&hcsr04_module_lock);
    memcpy(buffer, local_hcsr04_data, sizeof(uint16_t) * NUM_ULTRASONIC_SENSORS);
    pthread_mutex_unlock(&hcsr04_module_lock);
}

void register_hcsr04_callback(hcsr04_receive_data_callback callback)
{
  if (callbacks_count == MAX_HCSR04_CALLBACKS)
  {
     mikes_log(ML_ERR, "too many hcsr04 callbacks");
     return;
  }
  callbacks[callbacks_count] = callback;
  callbacks_count++;
}

void unregister_hcsr04_callback(hcsr04_receive_data_callback callback)
{
  for (int i = 0; i < callbacks_count; i++)
    if (callbacks[i] == callback)
    {
       callbacks[i] = callbacks[callbacks_count - 1];
       callbacks_count--;
    }
}

void shutdown_hcsr04()
{
    printf("shutdown_hcsr04()\n");
    hcsr04_initialized = 0;
    close(fdR[1]);
    close(fdW[0]);
    usleep(10000);
    kill(plink_child, SIGTERM);
}
