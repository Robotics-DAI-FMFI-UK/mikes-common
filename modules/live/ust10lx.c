#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "../../bites/mikes.h"
#include "ust10lx.h"
#include "../passive/mikes_logs.h"
#include "core/config_mikes.h"

#define BUFFER_SIZE 5000
#define MAX_ERROR_RAYS 5
#define MAX_NEIGHBOR_DIFF 60

#define MAX_UST10LX_CALLBACKS 20

static pthread_mutex_t ust10lx_lock;

uint16_t *ust10lx_data;

static uint16_t *local_data;
static int sockfd;

static ust10lx_receive_data_callback callbacks[MAX_UST10LX_CALLBACKS];
static int callbacks_count;

static int online;

void connect_ust10lx()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
    mikes_log(ML_ERR, "cannot open ust-10lx socket");
    perror("mikes:ust10lx");
        return;
    }

    struct sockaddr_in remoteaddr;
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_addr.s_addr = inet_addr(UST10LX_ADDR);
    remoteaddr.sin_port = htons(UST10LX_PORT);

    if (connect(sockfd, (struct sockaddr*)&remoteaddr, sizeof(remoteaddr)) < 0)
    {
    mikes_log(ML_ERR, "connecting ust-10lx socket");
    perror("mikes:ust10lx");
        return;
    }

    mikes_log(ML_INFO, "UST-10LX connected");
}

void *ust10lx_thread(void *args)
{
    char *start_measurement = "BM\n";
    char *request_measurement = "GD0000108000\n";
    unsigned char readbuf[BUFFER_SIZE];

    if (write(sockfd, start_measurement, strlen(start_measurement)) < 0)
    {
        perror("mikes:ust10lx");
        mikes_log(ML_ERR, "writing start measurement packet to ust-10lx");
    }

    usleep(250000);

    unsigned char x[2];
    int cnt = 0;
    do {
        if (read(sockfd, x + ((cnt++) % 2), 1) < 0)
        {
            perror("mikes:ust10lx");
            mikes_log(ML_ERR, "reading response from ust10-lx");
            break;
        }
    } while ((x[0] != 10) || (x[1] != 10));

    while (program_runs)
    {
        if (write(sockfd, request_measurement, strlen(request_measurement)) < 0)
        {
            perror("mikes:ust10lx");
            mikes_log(ML_ERR, "writing request to ust-10lx");
            break;
        }

        int readptr = 0;
        do {
            int nread = read(sockfd, readbuf + readptr, BUFFER_SIZE - readptr);
            if (nread < 0)
            {
                perror("mikes:ust10lx");
                mikes_log(ML_ERR, "reading response from ust-10lx");
                break;
            }
            readptr += nread;
            if (readptr < 2) continue;
        } while ((readbuf[readptr - 1] != 10) || (readbuf[readptr - 2] != 10));

        int searchptr = 0;
        for (int i = 0; i < 3; i++)
        {
            while ((readbuf[searchptr] != 10) && (searchptr < readptr))
                searchptr++;
            searchptr++;
        }

        if (readptr - searchptr != 103 + UST10LX_DATA_COUNT * 3)
        {
            static char *logmsg1 = "Hokuyo returned packet of unexpected size, I will ignore it size=%d";
            char msg[strlen(logmsg1) + 20];
            sprintf(msg, logmsg1, readptr - searchptr);
            mikes_log(ML_WARN, msg);
            continue;
        }

        int beam_index = UST10LX_DATA_COUNT - 1;
        readptr = searchptr;
        while (beam_index >= 0)
        {
            int pos = (searchptr - readptr) % 66;
            if (pos == 62)
            {
                local_data[beam_index] = ((readbuf[searchptr] - 0x30) << 12) |
                                         ((readbuf[searchptr + 1] - 0x30) << 6) |
                                         (readbuf[searchptr + 4] - 0x30);
                searchptr += 5;
            } else if (pos == 63)
            {
                local_data[beam_index] = ((readbuf[searchptr] - 0x30) << 12) |
                                         ((readbuf[searchptr + 3] - 0x30) << 6) |
                                         (readbuf[searchptr + 4] - 0x30);
                searchptr += 5;
            } else
            {
                if (pos == 64) searchptr += 2;
                local_data[beam_index] = ((((uint16_t)readbuf[searchptr]) - 0x30) << 12) |
                                         ((((uint16_t)readbuf[searchptr + 1]) - 0x30) << 6) |
                                         (((uint16_t)readbuf[searchptr + 2]) - 0x30);
                searchptr += 3;
            }
            beam_index--;
        }

        pthread_mutex_lock(&ust10lx_lock);
        memcpy(ust10lx_data, local_data, sizeof(uint16_t) * UST10LX_DATA_COUNT);
        pthread_mutex_unlock(&ust10lx_lock);

        for (int i = 0; i < callbacks_count; i++)
          callbacks[i](local_data);

        usleep(25000);
    }

    mikes_log(ML_INFO, "ust10lx quits.");
    threads_running_add(-1);
    return 0;
}

void init_ust10lx()
{
    if (!mikes_config.use_ust10lx)
    {
        mikes_log(ML_INFO, "ust10lx supressed by config.");
        online = 0;
        return;
    }
    online = 1;

    pthread_t t;
    ust10lx_data = (uint16_t *) malloc(sizeof(uint16_t) * UST10LX_DATA_COUNT);
    local_data = (uint16_t *) malloc(sizeof(uint16_t) * UST10LX_DATA_COUNT);
    if ((ust10lx_data == 0) || (local_data == 0))
    {
      perror("mikes:ust10lx");
      mikes_log(ML_ERR, "insufficient memory");
      exit(1);
    }
    connect_ust10lx();
    pthread_mutex_init(&ust10lx_lock, 0);
    if (pthread_create(&t, 0, ust10lx_thread, 0) != 0)
    {
      perror("mikes:ust10lx");
      mikes_log(ML_ERR, "creating thread for ust-10lx");
    }
    else threads_running_add(1);
}

void get_ust10lx_data(uint16_t* buffer)
{
    if (!online) return;
    pthread_mutex_lock(&ust10lx_lock);
    memcpy(buffer, ust10lx_data, sizeof(uint16_t) * UST10LX_DATA_COUNT);
    pthread_mutex_unlock(&ust10lx_lock);
}

int ust10lx_ray2azimuth(uint16_t ray)
{
  return (360 + (UST10LX_TOTAL_ANGLE_DEG / 2 - ray / UST10LX_SIZE_OF_ONE_DEG)) % 360;
}

uint16_t ust10lx_azimuth2ray(int alpha)
{
  if (360 - alpha <= UST10LX_TOTAL_ANGLE_DEG / 2) alpha -= 360;
  if (alpha < -UST10LX_TOTAL_ANGLE_DEG / 2) alpha = -UST10LX_TOTAL_ANGLE_DEG / 2;
  else if (alpha > UST10LX_TOTAL_ANGLE_DEG / 2) alpha = UST10LX_TOTAL_ANGLE_DEG / 2;
  return UST10LX_DATA_COUNT / 2 - alpha * UST10LX_SIZE_OF_ONE_DEG;
}

void register_ust10lx_callback(ust10lx_receive_data_callback callback)
{
  if (!online) return;
  if (callbacks_count == MAX_UST10LX_CALLBACKS)
  {
     mikes_log(ML_ERR, "too many UST10LX callbacks");
     return;
  }
  callbacks[callbacks_count] = callback;
  callbacks_count++;
}

void unregister_ust10lx_callback(ust10lx_receive_data_callback callback)
{
  if (!online) return;
  for (int i = 0; i < callbacks_count; i++)
    if (callbacks[i] == callback)
    {
       callbacks[i] = callbacks[callbacks_count - 1];
       callbacks_count--;
    }
}

