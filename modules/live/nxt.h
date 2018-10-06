#ifndef __nxt_h__
#define __nxt_h__

#define PATH_TO_NXTOPERATOR "/home/pi/src/mikes-generic/mikes-common/nxt/NXTOperator"

#define NXT_KEY_LEFT 1
#define NXT_KEY_ENTER 2
#define NXT_KEY_RIGHT 3

void init_nxt();
void shutdown_nxt();

// turns the wheels on
void nxt_wheels_on();

// turns the wheels off 
void nxt_wheels_off();

// move the grabber to the first, second, or third row (ln = 1,2,3)
// after it has been docked before
void nxt_line(int ln);

// move the grabber back to home position
void nxt_dock();

// setup fast grabber speed
void nxt_fast();

// setup slow grabber speed
void nxt_slow();

// print message on NXT display
void nxt_message(const char *msg);

// clear NXT display
void nxt_clear_display();

// read key from NXT (Left, Enter, Right)
int nxt_read_key();

// returns 1 after the grabber movement have completed, otherwise 0
int nxt_done();

// returns 1 if NXT is connected, otherwise returns 0
int nxt_is_online();

#endif
