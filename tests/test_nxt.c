#include <stdio.h>
#include <unistd.h>

#include "../modules/live/nxt.h"
#include "../bites/mikes.h"

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
  printf("on\n");
  nxt_wheels_on();
  sleep(3);
  printf("off\n");
  nxt_wheels_off();
  sleep(3);
  printf("dock\n");
  nxt_dock();
  while (!nxt_done()) usleep(10000);
  printf("done\n");
  sleep(2);
  printf("line1\n");
  nxt_line(1);
  while (!nxt_done()) usleep(10000);
  printf("done\n");
  sleep(2);
  printf("dock\n");
  nxt_dock();
  while (!nxt_done()) usleep(10000);
  printf("done\n");
  sleep(2);
  printf("line2\n");
  nxt_line(2);
  while (!nxt_done()) usleep(10000);
  printf("done\n");
  sleep(2);
  printf("dock\n");
  nxt_dock();
  while (!nxt_done()) usleep(10000);
  printf("done\n");
  sleep(2);
  printf("line3\n");
  nxt_line(3);
  while (!nxt_done()) usleep(10000);
  printf("done\n");
  sleep(2);
  printf("dock\n");
  nxt_dock();
  while (!nxt_done()) usleep(10000);
  sleep(2);
  printf("line1 fast\n");
  nxt_fast();
  nxt_line(1);
  while (!nxt_done()) usleep(10000);
  printf("done\n");
  sleep(2);
  printf("dock slow\n");
  nxt_slow();
  nxt_dock();
  while (!nxt_done()) usleep(10000);
  printf("message\n");
  nxt_clear_display();
  nxt_message("hello");
  nxt_message("enter_key");
  int key = nxt_read_key();
  printf("key=%d\n", key);

  printf("shutdown()\n");

  shutdown_nxt();
  mikes_shutdown();
}
