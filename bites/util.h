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

int wait_for_new_data(int *fd);

int alert_new_data(int *fd);

// utility function to compute directional angle from alpha to beta (+/- 180), all values in deg
short angle_difference(short alpha, short beta);

// utility function to compute directional angle from alpha to beta (+/- 180), all values in rad
double angle_rad_difference(double alpha, double beta);

// change robot angle to map angle
double compass_heading_to_map_heading(double alpha);

// Change start of nagle from (1:0) to (0:1) and from anticlockwise to clockwise
double math_azimuth_to_robot_azimuth(double alpha);

#endif
