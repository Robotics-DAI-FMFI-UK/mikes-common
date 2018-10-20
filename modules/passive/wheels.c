#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "../../bites/mikes.h"
#include "mikes_logs.h"
#include "../live/base_module.h"
#include "wheels.h"

static int fdR[2];
static int fdW[2];
static pid_t plink_child;
static unsigned char wheels_initialized;

void connect_wheels()
{
    if (pipe(fdR) < 0)
    {
        mikes_log(ML_ERR, "wheels: pipe2()");
        wheels_initialized = 0;
        return;
    }
    if (pipe(fdW) < 0)
    {
        mikes_log(ML_ERR, "wheels: pipe2()");
        wheels_initialized = 0;
        return;
    }

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

        if (execl("/usr/bin/plink", "/usr/bin/plink", "/dev/grabber",
                  "-serial", "-sercfg", "115200,N,n,8,1", NULL) < 0)
        {
            mikes_log(ML_ERR, "wheels: child execl()");
            wheels_initialized = 0;
            return;
        }
    }

    if (plink_child < 0)
    {
        mikes_log(ML_ERR, "wheels: child execl()");
        wheels_initialized = 0;
        return;
    }

    close(fdR[0]);
    close(fdW[1]);
    if (fcntl( fdW[0], F_SETFL, fcntl(fdW[0], F_GETFL) | O_NONBLOCK) < 0)
    {
        mikes_log(ML_ERR, "wheels: setting nonblock on read pipe end");
        wheels_initialized = 0;
    }

    mikes_log(ML_INFO, "wheels module connected");
    sleep(2); // arduino power up 
    wheels_initialized = 1;
}

void wheels_send_cmd(char *cmd)
{
    if (!wheels_initialized) return;
    if (write(fdR[1], cmd, 1) < 1)
    {
       perror("mikes:wheels");
       mikes_log(ML_ERR, "wheels: could not send command");
    }
}

void wheels_up()
{
    wheels_send_cmd("U");
}

void wheels_down()
{
    wheels_send_cmd("D");
}

void wheels_forward()
{
    wheels_send_cmd("F");
}

void wheels_test()
{
    wheels_send_cmd("T");
}

void cargo_unload()
{
    wheels_send_cmd("O");
    sleep(3);
    unloading_shake();
    sleep(3);
    unloading_shake();
    sleep(3);
    wheels_send_cmd("C");
}

void init_wheels()
{
    wheels_initialized = 0;
    connect_wheels();
}

void shutdown_wheels()
{
    printf("shutdown_wheels()\n");
    close(fdR[1]);
    close(fdW[0]);
    usleep(10000);
    kill(plink_child, SIGTERM);
}

int wheels_obstacle()
{
    char o;
    wheels_send_cmd("V");
    int cnt = 0;
    while ((cnt++ < 20) && (read(fdW[0], &o, 1) != 1)) usleep(1000); 
    if (o == 'P') return 1;
    return 0;
}

