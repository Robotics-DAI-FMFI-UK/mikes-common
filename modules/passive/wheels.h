#ifndef __WHEELS_H__
#define __WHEELS_H__

// must be called first
void init_wheels();

// move the wheels up (usual position when navigating)
void wheels_up();

// move the wheels down to grabbing position
void wheels_down();

// move the wheels forward (just in case)
void wheels_forward();

// test all positions
void wheels_test();

#endif
