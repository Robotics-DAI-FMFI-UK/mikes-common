#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../modules/passive/actuator.h"
#include "../modules/passive/wheels.h"
#include "../modules/live/base_module.h"
#include "../modules/live/nxt.h"
#include "../bites/mikes.h"
#include "../core/config_mikes.h"
#include "../bites/util.h"

#define ONLY_UNLOAD 1
#define ALL_TESTS 2

void test_actuator(int tests)
{
    if (tests == ALL_TESTS)
    {
      say("grabbing the balls");
      sleep(1);
  
      grab_line(1);
      sleep(2);
      grab_line(2);
      sleep(2);
      grab_line(3);
      sleep(1);
    }

    say("unloading");
    unload_cargo();
}

int main(int argc, char **argv)
{
  int tests = ALL_TESTS;

  mikes_init(argc, argv);
  if (argc > 1)
  {
    if (strcmp(argv[1], "unloading") == 0)
      tests = ONLY_UNLOAD;
    else tests = ALL_TESTS;
  }

  printf("init()\n");
  if (tests == ALL_TESTS)
  {
    init_nxt();
    sleep(3);
    if (!nxt_is_online())
    {
      printf("nxt is not connected\n");
      shutdown_nxt();
      mikes_shutdown();
      return 0;
    }
  }
  init_base_module();

  init_wheels();
  sleep(2);

  init_actuator();
  sleep(1);

  test_actuator(tests);

  shutdown_nxt();
  shutdown_wheels();

  mikes_shutdown();
}

