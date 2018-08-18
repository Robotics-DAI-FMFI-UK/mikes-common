#ifndef _UTIL_H_
#define _UTIL_H_

// square of distance of two points
//double distance(double x1, double y1, double x2, double y2);

// return current time in milliseconds
long long msec();

// return current time in usec 
long long usec();

// say the sentence
void say(char *sentence);

double normAlpha(double alpha);

double rad_normAlpha(double alpha);

#endif
