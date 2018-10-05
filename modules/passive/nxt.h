#ifndef __nxt_h__
#define __nxt_h__

#include <stdint.h>

#define PATH_TO_NXTOPERATOR "/home/pi/src/mikes-generic/mikes-common/nxt/NXTOperator"

void init_nxt();
void shutdown_nxt();

// turns all motors on to desired speed
void nxt_on(int8_t speed);

// swich off all motor power
void nxt_off();

// active brake on all motors
void nxt_brake();

#endif
