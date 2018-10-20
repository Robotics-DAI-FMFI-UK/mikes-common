#include <stdio.h>
#include <unistd.h>

#include "../modules/live/nxt.h"
#include "../bites/mikes.h"

#define CHOICE_ON      '1'
#define CHOICE_OFF     '2'
#define CHOICE_DOCK    '3'
#define CHOICE_LINE1   '4'
#define CHOICE_LINE2   '5'
#define CHOICE_LINE3   '6'
#define CHOICE_FAST    '7'
#define CHOICE_SLOW    '8'
#define CHOICE_MESSAGE '9'
#define CHOICE_KEY     '0'
#define CHOICE_QUIT    'q'

void wait_till_done()
{
  while (!nxt_done()) usleep(20000);
}

char menu()
{
  char c;
  printf("1   wheels on\n");
  printf("2   wheels off\n");
  printf("3   dock\n");
  printf("4   line 1\n");
  printf("5   line 2\n");
  printf("6   line 3\n");
  printf("7   fast\n");
  printf("8   slow\n");
  printf("9   message\n");
  printf("0   key\n");
  printf("q   quit\n");
  printf("\nYour choice:");
  do { scanf("%c", &c); } while (((c < '0') || (c > '9')) && (c != 'q'));
  return c;
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
  char choice;
  do {
    choice = menu();
    int key;
    switch (choice)
    {
     case CHOICE_ON: printf("on\n");
                     nxt_wheels_on();
                     break;
     case CHOICE_OFF: printf("off\n");
                      nxt_wheels_off();
                      break;
     case CHOICE_DOCK: printf("dock\n");
                       nxt_dock();
                       wait_till_done();
                       printf("done\n");
                       break;
     case CHOICE_LINE1: printf("line1\n");
                        nxt_line(1);
                        wait_till_done();
                        printf("done\n");
                        break;
     case CHOICE_LINE2: printf("line2\n");
                        nxt_line(2);
                        wait_till_done();
                        printf("done\n");
                        break;
     case CHOICE_LINE3: printf("line3\n");
                        nxt_line(3);
                        wait_till_done();
                        printf("done\n");
                        break;
     case CHOICE_FAST:  nxt_fast();
                        break;
     case CHOICE_SLOW:  nxt_slow();
                        break;
     case CHOICE_MESSAGE: printf("message\n");
                          nxt_clear_display();
                          usleep(50000);
                          nxt_message("hello");
                          usleep(50000);
                          nxt_message("how r u?");
                          break;
     case CHOICE_KEY: key = nxt_read_key();
                      printf("key=%d\n", key);
                      break;
    }
  } while (choice != CHOICE_QUIT);

  shutdown_nxt();
  mikes_shutdown();
}
