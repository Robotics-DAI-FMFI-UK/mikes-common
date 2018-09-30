#include <stdio.h>
#include <unistd.h>
#include "../modules/passive/nxt/nxt.h"

int main()
{
  printf("init()\n");
  init_nxt();
  sleep(3);

  printf("on(30)\n");
  nxt_on(30);
  sleep(3);
  printf("off()\n");
  nxt_off();
  sleep(3);
  printf("on(-30)\n");
  nxt_on(-30);
  sleep(3);
  printf("brake()\n");
  nxt_brake();
  sleep(2);
  printf("off()\n");
  nxt_off();

  printf("shutdown()\n");
  shutdown_nxt();
}
