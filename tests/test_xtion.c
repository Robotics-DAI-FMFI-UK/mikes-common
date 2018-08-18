#include <stdio.h>
#include <unistd.h>

#include "../core/config_mikes.h"
#include "../modules/live/xtion/xtion.h"
#include "../bites/mikes.h"

int main(int argc, char **argv)
{
  short data[16 * 12];

  mikes_init(argc, argv);
  init_xtion(16, 12);

  sleep(1);
  get_xtion_data(data);
  for (int r = 0; r < 12; r++)
  {
    for (int c = 0; c < 16; c++)
      printf("%5d", data[r * 16 + c]);
    printf("\n");
  }

  mikes_shutdown();
}
