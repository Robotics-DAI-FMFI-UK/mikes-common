#ifndef _MIKES_DAEMON_H_
#define _MIKES_DAEMON_H_

extern volatile unsigned char program_runs;

void threads_running_add(short x);

void mikes_init(int argc, char **argv);
void mikes_shutdown();

#endif
