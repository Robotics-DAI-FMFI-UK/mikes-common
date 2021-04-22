#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../modules/live/hcsr04.h"
#include "../modules/passive/mikes_logs.h"
#include "../bites/mikes.h"
#include "../core/config_mikes.h"
#include "../bites/util.h"

void new_us_data(hcsr04_data_type new_data)
{
    printf("us: %hu %hu %hu %hu %hu %hu %hu %hu\n",
    new_data[0], new_data[1], new_data[2], 
    new_data[3], new_data[4], new_data[5], 
    new_data[6], new_data[7]);
}

void test_hcsr04()
{
  hcsr04_data_type hcsr04_data;
  
  printf("reading/printing US sensors, 30 seconds, 1 Hz...\n");
  for (int i = 0; i < 30; i++)
  {
    get_hcsr04_data(hcsr04_data);
    printf("us: %hu %hu %hu %hu %hu %hu %hu %hu\n",
    hcsr04_data[0], hcsr04_data[1], hcsr04_data[2], 
    hcsr04_data[3], hcsr04_data[4], hcsr04_data[5], 
    hcsr04_data[6], hcsr04_data[7]);
    sleep(1);
  }
  printf("registering callback, and printing arrived data for 10s...\n");
  
  register_hcsr04_callback(new_us_data);
  sleep(10);
  unregister_hcsr04_callback(new_us_data);
    
  printf("test done.\n");
}

int main(int argc, char **argv)
{
  mikes_init(argc, argv);
  printf("mikes initialized\n");
  init_hcsr04();
  printf("testing\n");
  test_hcsr04();

  shutdown_hcsr04();
  mikes_shutdown();
}

