#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "core/mikes.h"
#include "modules/passive/mikes_logs.h"
#include "core/config_mikes.h"
#include "modules/live/base_module.h"
#include "modules/passive/pose.h"
#include "bites/util.h"

volatile unsigned char program_runs;
static pthread_mutex_t mikes_lock;
volatile unsigned short threads_running;

void threads_running_add(short x)
{
  pthread_mutex_lock(&mikes_lock);
  threads_running += x;
  pthread_mutex_unlock(&mikes_lock);
}

void signal_term_handler(int signum)
{
  program_runs = 0;
  printf("TERMINATING...\n");
}

void say_greeting()
{
  say("Hello");
  sleep(1);
  say("my name is");
  sleep(1);
  say("me cash.");
  sleep(1);
  say("How do you do?");
}

void mikes_init(int argc, char **argv)
{
  program_runs = 1;
  threads_running = 1;
  pthread_mutex_init(&mikes_lock, 0);
  signal(SIGTERM, signal_term_handler);

  load_config();
  init_mikes_logs();
  say_greeting();
}
  

void mikes_shutdown()
{
  program_runs = 0;
  int old_tr = threads_running + 1;
  while (threads_running > 1)
  {
    usleep(10000);
    if (threads_running < old_tr)
    {
      char tr[50];
      sprintf(tr, "%d threads running", threads_running);
      mikes_log(ML_INFO, tr);
      old_tr = threads_running;
    }
  }

  mikes_log(ML_INFO, "Kocur mikes quits.");
  usleep(100000);
}

