#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <libxml/xmlreader.h>
#include <sys/time.h>

#include "../modules/passive/mikes_logs.h"
#include "core/config_mikes.h"
#include "util.h"


/* double distance(double x1, double y1, double x2, double y2)
{
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
} */

long long msec()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return 1000L * tv.tv_sec + tv.tv_usec / 1000L;
}

long long usec()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (1000000L * (long long)tv.tv_sec) + tv.tv_usec;
}

void say(char *sentence)
{
    time_t current_time;
    time(&current_time);
    char buf[128];
    sprintf(buf, "/bin/bash -c \"echo '%s' | espeak -v en-us -p 90 -a 400 2>/dev/null >/dev/null & \"", sentence);
    system(buf);
}

double normAlpha(double alpha){
   if(alpha < 0){
      while(alpha < 0)
         alpha += 360;
   }
   else
      while(alpha >= 360)
         alpha -= 360;
   return alpha;
}

double rad_normAlpha(double alpha){
   if(alpha < 0){
      while(alpha < 0)
         alpha += 2.0 * M_PI;
   }
   else
      while(alpha >= 2.0 * M_PI)
         alpha -= 2.0 * M_PI;
   return alpha;
}

short angle_difference(short alpha, short beta)
{
  short diff = beta - alpha;
  if (diff > 180) return diff - 360;
  else if (diff < -180) return diff + 360;
  return diff;
}

double angle_rad_difference(double alpha, double beta)
{
  beta = rad_normAlpha(beta);
  alpha = rad_normAlpha(alpha);
  double diff = beta - alpha;
  if (diff > M_PI) return diff - 2.0 * M_PI;
  else if (diff < -M_PI) return diff + 2.0 * M_PI;
  return diff;
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------PIPES-------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

static char sendbuffer[1] = {'G'};
static char readbuffer[10];

int wait_for_new_data(int *fd)
{
  return read(fd[0], readbuffer, sizeof(readbuffer));
}

int alert_new_data(int *fd)
{
  return write(fd[1], sendbuffer, sizeof(sendbuffer));
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ---------------------------MAP ALPHA----------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

double compass_heading_to_map_heading(double alpha)
{
  return normAlpha(alpha + mikes_config.map_azimuth);
}
