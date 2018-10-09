#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "actuator.h"
#include "mikes_logs.h"
#include "../../bites/util.h"
#include "../live/nxt.h"
#include "../passive/wheels.h"

void init_actuator()
{
  nxt_dock();
  wheels_up();
}


void grab_line(int line)
{
  nxt_dock();
  nxt_line(line);
  sleep(4);
  nxt_wheels_on();
  wheels_down();
  sleep(2);
  nxt_dock();
  sleep(4);
  nxt_wheels_off();
  wheels_up();
  sleep(2);
}

void unload_cargo()
{
  cargo_unload();
}

