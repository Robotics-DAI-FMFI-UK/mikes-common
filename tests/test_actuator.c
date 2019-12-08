#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../modules/passive/actuator.h"
#include "../modules/passive/wheels.h"
#include "../modules/passive/mikes_logs.h"
#include "../modules/live/base_module.h"
#include "../modules/live/nxt.h"
#include "../bites/mikes.h"
#include "../core/config_mikes.h"
#include "../bites/util.h"

#define ONLY_UNLOAD 1
#define ALL_TESTS 2

void test_actuator(int tests)
{
    long long t1 = usec();
    if (tests == ALL_TESTS)
    {
      say("grabbing the balls");
  
      grab_line(1);
      grab_line(2);
      grab_line(3);
    }
    printf("grabbing time: %.3lf s\n", (usec() - t1) / 1000000.0);

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

  printf("mikes initialized\n");
  init_wheels();

  if (tests == ALL_TESTS)
  {
    init_nxt();
    sleep(5);
    if (!nxt_is_online())
    {
      printf("nxt is not connected\n");
      mikes_log(ML_ERR, "nxt not online");
      shutdown_wheels();
      shutdown_nxt();
      mikes_shutdown();
      return 0;
    }
  }
  printf("nxt initialized\n");
  init_base_module();
  sleep(2);

  init_actuator();
  sleep(1);

  printf("testing\n");
  test_actuator(tests);

  shutdown_nxt();
  shutdown_wheels();

  mikes_shutdown();
}

