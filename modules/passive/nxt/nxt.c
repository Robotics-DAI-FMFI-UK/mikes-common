#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "nxt.h"

static int fd[2];
static int opened = 0;

void init_nxt()
{
  if (opened) return;
  if (pipe(fd))
  {
    perror("nxt module pipe()");
    return;
  }
  if (fork() == 0)
  {
    close(fd[1]);
    dup2(fd[0], 0);
    close(fd[0]);
    execl(PATH_TO_NXTOPERATOR, PATH_TO_NXTOPERATOR, 0);
  }
  close(fd[0]);
}

void shutdown_nxt()
{
  if (write(fd[1], "Quit\n", 4) < 0)
    perror("nxt write() to pipe");
  usleep(200000);
  close(fd[1]); 
}

void nxt_on(int8_t speed)
{
  char num[6];

  if (write(fd[1], "On\n", 3) < 0)
    perror("nxt write() on pipe");
  else
  {
    sprintf(num, "%d\n", speed);
    if (write(fd[1], num, strlen(num)) < 0)
      perror("nxt on pipe write()");
  }
}

void nxt_off()
{
  if (write(fd[1], "Off\n", 4) < 0)
    perror("nxt pipe write()");
}

void nxt_brake()
{
  if (write(fd[1], "Brake\n", 6) < 0)
    perror("pipe nxt write()");
}

