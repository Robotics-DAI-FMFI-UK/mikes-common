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

void wait_till_done()
{
  while (!nxt_done()) usleep(20000);
}

void init_actuator()
{
  nxt_dock();
  wait_till_done();
  wheels_up();
}

void grab_line(int line)
{
  mikes_log_val(ML_INFO, "actuator: grabbing line ", line);

  mikes_log(ML_INFO, "actuator: dock");
  nxt_dock();
  wait_till_done();
  mikes_log(ML_INFO, "actuator: docked");

  mikes_log(ML_INFO, "actuator: line");
  nxt_line(line);
  wait_till_done();
  mikes_log(ML_INFO, "actuator: on line");
  nxt_wheels_on();
  mikes_log(ML_INFO, "actuator: wheels are on");
  wheels_down();
  mikes_log(ML_INFO, "actuator: wheels are going down, sleep 2");
  sleep(2);
  mikes_log(ML_INFO, "actuator: dock");
  nxt_dock();
  wait_till_done();
  mikes_log(ML_INFO, "actuator: docked, turning off wheels");
  nxt_wheels_off();
  mikes_log(ML_INFO, "actuator: moving wheels up");
  usleep(300000);
  wheels_up();
  mikes_log(ML_INFO, "actuator: done grabbing line");
}

void unload_cargo()
{
  mikes_log(ML_INFO, "actuator: unloading cargo");
  cargo_unload();
}

