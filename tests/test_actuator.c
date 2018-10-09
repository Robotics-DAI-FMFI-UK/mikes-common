#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../modules/passive/actuator.h"
#include "../modules/passive/wheels.h"
#include "../modules/live/nxt.h"
#include "../bites/mikes.h"
#include "../core/config_mikes.h"
#include "../bites/util.h"

void test_actuator()
{
    say("grabbing the balls");
    sleep(1);

    grab_line(1);
    sleep(2);
    grab_line(2);
    sleep(2);
    grab_line(3);
    sleep(1);
  
    say("unloading");
    unload_cargo();
}

int main(int argc, char **argv)
{
  mikes_init(argc, argv);

  printf("init()\n");
  init_nxt();
  sleep(3);
  if (!nxt_is_online())
  {
    printf("nxt is not connected\n");
    shutdown_nxt();
    mikes_shutdown();
    return 0;
  }

  init_wheels();
  sleep(2);

  init_actuator();

  test_actuator();

  shutdown_nxt();

  mikes_shutdown();
}

