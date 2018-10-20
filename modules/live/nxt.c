#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include "../../bites/mikes.h"
#include "../passive/mikes_logs.h"

#include "nxt.h"

static int fd[2];
static int infd[2];
static volatile int opened = 0;
static volatile int key;
static volatile int nxt_online;
static volatile int nxt_command_done;
static volatile int nxt_key_arrived;

void process_response(char *buf)
{
  mikes_log_str(ML_DEBUG, "nxt response: ", buf);
  if (strcmp(buf, "Start") == 0) { nxt_online = 1; mikes_log(ML_INFO, "nxt is online"); }
  else if (strcmp(buf, "Stop") == 0) nxt_online = 0;
  else if (strcmp(buf, "Done") == 0) nxt_command_done = 1;
  else if (strcmp(buf, "Off") == 0) return;
  else if (strcmp(buf, "Left") == 0) { key = NXT_KEY_LEFT; nxt_key_arrived = 1; }
  else if (strcmp(buf, "Enter") == 0) { key = NXT_KEY_ENTER; nxt_key_arrived = 1; }
  else if (strcmp(buf, "Right") == 0) { key = NXT_KEY_RIGHT; nxt_key_arrived = 1; }
  else printf("nxt: unexpected response '%s'\n", buf);
}

void *nxt_thread(void *args)
{
    char buf[50];
    char *bufp = buf;
    char b;

    mikes_log(ML_INFO, "nxt_thread started");
    while (program_runs)
    {
      if (read(infd[0], &b, 1) == 1)
      {
        *(bufp++) = b;
        if (b == '\n') 
        {
          *(--bufp) = 0;
          bufp = buf;
          process_response(buf);
        }
      }
      else usleep(10000);
    }

    printf("nxt quits\n");
    threads_running_add(-1);
    return 0;
}

void init_nxt()
{
  mikes_log(ML_INFO, "nxt: init");
  if (opened) return;
  nxt_online = 0;
  nxt_command_done = 0;
  nxt_key_arrived = 0;
  if (pipe(fd))
  {
    perror("nxt module pipe()");
    return;
  }
  if (pipe(infd))
  {
    perror("nxt module pipe()");
    return;
  }
  if (fork() == 0)
  {
    close(fd[1]);
    dup2(fd[0], 0);
    close(fd[0]);
    close(infd[0]);
    dup2(infd[1], 1);
    close(infd[1]);
    execl(PATH_TO_NXTOPERATOR, PATH_TO_NXTOPERATOR, 0);
  }
  close(fd[0]);
  close(infd[1]);

  pthread_t t;
  if (pthread_create(&t, 0, nxt_thread, 0) != 0)
  {
      perror("mikes:nxt");
      printf("creating thread for nxt module");
      mikes_log(ML_ERR, "nxt: creating thread");
  }
  else threads_running_add(1);
  opened = 1;
  mikes_log(ML_INFO, "nxt: init ok");
}

void shutdown_nxt()
{
  mikes_log(ML_INFO, "nxt: shutdown");
  if (opened) 
  {
    if (write(fd[1], "Quit\n", 5) < 0)
    {
      perror("nxt write() to pipe");
      mikes_log(ML_ERR, "nxt: err pipe on shutdown");
    }
    printf("nxt: quit sent\n");
    usleep(200000);
    close(fd[1]); 
    close(infd[0]);
    opened = 0;
    mikes_log(ML_INFO, "nxt: shutdown ok");
  }
  else mikes_log(ML_WARN, "nxt: shutdown not opened");
}

int nxt_done()
{
  return nxt_command_done;
}

int nxt_is_online()
{
  return nxt_online;
}

// turns the wheels on
void nxt_wheels_on()
{
  if (!opened) return;
  if (!nxt_online) return;
  if (write(fd[1], "On\n", 3) < 0)
    perror("nxt write() on pipe");
}

// turns the wheels off
void nxt_wheels_off()
{
  if (!opened) return;
  if (!nxt_online) return;
  if (write(fd[1], "Off\n", 4) < 0)
    perror("nxt write() on pipe");
}

// move the grabber to the first, second, or third row (ln = 1,2,3)
// after it has been docked before
void nxt_line(int ln)
{
  char line[7];
  nxt_command_done = 0;

  mikes_log_val(ML_INFO, "nxt: line", ln);

  if (!opened) 
  {
    mikes_log(ML_WARN, "nxt: not opened");
    return;
  }
  if (!nxt_online) 
  {
    mikes_log(ML_WARN, "nxt: not online");
    return;
  }
  if ((ln > 3) || (ln < 1)) 
  {
    mikes_log(ML_ERR, "nxt: line number error");
    return;
  }
  sprintf(line, "Line%d\n", ln);
  if (write(fd[1], line, 6) < 0)
  {
    perror("nxt write() on pipe");
    mikes_log(ML_ERR, "nxt: write on pipe");
  }
}

// move the grabber back to home position
void nxt_dock()
{
  mikes_log(ML_INFO, "nxt: dock");
  if (!opened) 
  {
    mikes_log(ML_WARN, "nxt: not opened");
    return;
  }
  if (!nxt_online) 
  {
    mikes_log(ML_WARN, "nxt: not online");
    return;
  }
  nxt_command_done = 0;
  if (write(fd[1], "Dock\n", 5) < 0)
  {
    perror("nxt write() on pipe");
    mikes_log(ML_ERR, "nxt: write on pipe");
  }
}

// setup fast grabber speed
void nxt_fast()
{
  mikes_log(ML_INFO, "nxt: fast");
  if (!opened) return;
  if (!nxt_online) return;
  if (write(fd[1], "Fast\n", 5) < 0)
  {
    perror("nxt write() on pipe");
    mikes_log(ML_ERR, "nxt: write on pipe");
  }
}

// setup slow grabber speed
void nxt_slow()
{
  mikes_log(ML_INFO, "nxt: slow");
  if (!opened) return;
  if (!nxt_online) return;
  if (write(fd[1], "Slow\n", 5) < 0)
  {
    perror("nxt write() on pipe");
    mikes_log(ML_ERR, "nxt: write on pipe");
  }
}

// print message on NXT display
void nxt_message(const char *msg)
{
  mikes_log(ML_INFO, "nxt: message");
  if (!opened) return;
  if (!nxt_online) return;
  if (write(fd[1], msg, strlen(msg)) < 0)
  {
    perror("nxt write() on pipe");
    mikes_log(ML_ERR, "nxt: write on pipe");
  }
  if (write(fd[1], "\n", 1) < 0)
  {
    perror("nxt write() on pipe");
    mikes_log(ML_ERR, "nxt: write 2 on pipe");
  }
}

// clear NXT display
void nxt_clear_display()
{
  mikes_log(ML_INFO, "nxt: clear display");
  if (!opened) return;
  if (!nxt_online) return;
  if (write(fd[1], "Clear\n", 6) < 0)
  {
    perror("nxt write() on pipe");
    mikes_log(ML_ERR, "nxt: write on pipe");
  }
}

// read key from NXT (Left, Enter, Right)
int nxt_read_key()
{
  mikes_log(ML_INFO, "nxt: read key");
  if (!opened) return 0;
  if (!nxt_online) return 0;
  nxt_key_arrived = 0;
  if (write(fd[1], "Key\n", 4) < 0)
  {
    perror("nxt write() on pipe");
    mikes_log(ML_ERR, "nxt: write on pipe");
  }
  while (program_runs && !nxt_key_arrived) usleep(10000);
  mikes_log_val(ML_INFO, "nxt: key:", key);
  return key;
}

