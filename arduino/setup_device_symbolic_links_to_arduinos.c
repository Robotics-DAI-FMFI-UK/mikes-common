// Since Arduinos are of the very same type, we have to distinguish between them
// through opening port and reading a response (wheels arduino responds 'M' to 'M')
// and assing the device symbolic links on system start. 
// This program should be called from /etc/rc.local. 
// It creates links /dev/base and /dev/grabber

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define DEVICE_TRY1 "/dev/ttyUSB0"
#define DEVICE_TRY2 "/dev/ttyUSB1"

#define WAIT_UNTIL_LAZY_ARDUINO_RESETS_AFTER_PORT_IS_OPEN 2000

static int fdR[2];
static int fdW[2];
static pid_t plink_child;

int connect_arduino(char *device)
{
    if (pipe(fdR) < 0) return 0;
    if (pipe(fdW) < 0) return 0;
    if ((plink_child = fork()) == 0)
    {
        close(0);
        close(1);
        dup2(fdR[0], 0);
        dup2(fdW[1], 1);
        close(fdR[0]);
        close(fdR[1]);
        close(fdW[0]);
        close(fdW[1]);

        if (execl("/usr/bin/plink", "/usr/bin/plink", device,
                  "-serial", "-sercfg", "115200,N,n,8,1", NULL) < 0)
            return 0;
    }

    if (plink_child < 0) return 0;
    close(fdR[0]);
    close(fdW[1]);

    if (fcntl( fdW[0], F_SETFL, fcntl(fdW[0], F_GETFL) | O_NONBLOCK) < 0) return 0;
    return 1;
}

void assign_links(int i)
{
  if (i == 1)
  {
    symlink(DEVICE_TRY1, "/dev/grabber");
    symlink(DEVICE_TRY2, "/dev/base");
    printf("found grabber on %s, assinging base module to %s\n", DEVICE_TRY1, DEVICE_TRY2);
  }
  else
  {
    symlink(DEVICE_TRY2, "/dev/grabber");
    symlink(DEVICE_TRY1, "/dev/base");
    printf("did not find grabber on %s, assuming it is on %s\n", DEVICE_TRY1, DEVICE_TRY2);
  }
}

int main()
{
  char b;
  if (!connect_arduino(DEVICE_TRY1))
  {
    perror("arduino setup: could not connect");
    if (plink_child > 0) kill(plink_child, 15);
    return 0;
  }
  usleep(1000 * WAIT_UNTIL_LAZY_ARDUINO_RESETS_AFTER_PORT_IS_OPEN);
  if (write(fdR[1], "MMM", 4) < 0)
    perror("write");
  usleep(100000);
  int counter = 0;
  while ((counter < 100) && (read(fdW[0], &b, 1) != 1)) { counter++; usleep(10000); }
  if (b == 'M') assign_links(1);
  else assign_links(2);
  kill(plink_child, 15);
  return 0;
}
