#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <libxml/xmlreader.h>
#include <sys/time.h>

#include "../modules/passive/mikes_logs.h"
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

