#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "t265.h"

#include <librealsense2/h/rs_pipeline.h>
#include <librealsense2/h/rs_option.h>
#include <librealsense2/h/rs_frame.h>

#include "../../bites/mikes.h"
#include "../passive/mikes_logs.h"
#include "core/config_mikes.h"

#define MAX_T265_CALLBACKS 20

static pthread_mutex_t t265_lock;

static t265_pose_type local_pose_data;

static t265_receive_data_callback callbacks[MAX_T265_CALLBACKS];
static int callbacks_count;

static int online;

void rs_error(rs2_error* e)
{
    if (e)
    {
        char s[300];
        snprintf(s, 300, "rs_error was raised when calling %s(%s): %s", rs2_get_failed_function(e), rs2_get_failed_args(e),
                         rs2_get_error_message(e));
        mikes_log(ML_ERR, s);
        printf("%s\n", s);
        exit(EXIT_FAILURE);
    }
}

void *t265_thread(void *args)
{
    rs2_error* e = 0;

    rs2_context* ctx = rs2_create_context(RS2_API_VERSION, &e);
    rs_error(e);

    rs2_pipeline* pipeline =  rs2_create_pipeline(ctx, &e);
    rs_error(e);
    rs2_config* config = rs2_create_config(&e);
    rs_error(e);

    // Add pose stream

    rs2_pose *pose_frame_data;

    rs2_config_enable_stream(config, RS2_STREAM_POSE, 0, 0, 0, RS2_FORMAT_6DOF, 0, &e);
    rs_error(e);

    rs2_pipeline_profile* pipeline_profile = rs2_pipeline_start_with_config(pipeline, config, &e);
    rs_error(e);

    while (program_runs)
    {
        rs2_frame* frames = rs2_pipeline_wait_for_frames(pipeline, RS2_DEFAULT_TIMEOUT, &e);
        rs_error(e);
        rs2_frame* frame = rs2_extract_frame(frames, 0, &e);
        rs_error(e);
        pose_frame_data = (rs2_pose *)(rs2_get_frame_data(frame, &e));
        rs_error(e);

        pthread_mutex_lock(&t265_lock); 
          local_pose_data = *pose_frame_data;
        pthread_mutex_unlock(&t265_lock);
 
        rs2_release_frame(frame);
        rs2_release_frame(frames);
        
        double heading;
        get_t265_heading(&local_pose_data, &heading);
        for (int i =0; i< callbacks_count; i++)
            callbacks[i](&local_pose_data, &heading);
    }

    rs2_pipeline_stop(pipeline, &e);
    rs_error(e);

    rs2_delete_pipeline_profile(pipeline_profile);
    rs2_delete_config(config);
    rs2_delete_pipeline(pipeline);
    rs2_delete_context(ctx);

    mikes_log(ML_INFO, "t265 quits.");
    threads_running_add(-1);
    return 0;
}

void init_t265()
{
    if (!mikes_config.use_t265)
    { 
      mikes_log(ML_INFO, "t265 supressed by config.");
      online = 0;
      return;
    }
    online = 1;
	
    pthread_t t;
	
    callbacks_count = 0;
    pthread_mutex_init(&t265_lock, 0);
    if (pthread_create(&t, 0, t265_thread, 0) != 0)
    {	
      perror("mikes:t265");
      mikes_log(ML_ERR, "creating thread for t265 sensor");
    }
    else threads_running_add(1);
}

void register_t265_callback(t265_receive_data_callback callback)
{
    if (!online) return;

    if (callbacks_count == MAX_T265_CALLBACKS)
    {
      mikes_log(ML_ERR, "too many T265 callbacks");
      return;
    }
    callbacks[callbacks_count] = callback;
    callbacks_count++;
}

void unregister_t265_callback(t265_receive_data_callback callback)
{
    if (!online) return;

    for (int i = 0; i < callbacks_count; i++)
      if (callbacks[i] == callback)
      {
         callbacks[i] = callbacks[callbacks_count - 1];
         callbacks_count--;
      }
}

void get_t265_pose(t265_pose_type *pose)
{
    if (!online) return;

    pthread_mutex_lock(&t265_lock);
      *pose = local_pose_data;
    pthread_mutex_unlock(&t265_lock);
}

#define POSE_LOGSTR_LEN 200

void log_t265_pose(t265_pose_type *pose)
{
    char str[POSE_LOGSTR_LEN];
    double heading;

    get_t265_heading(pose, &heading);

    sprintf(str, "[main] pose::log_t265_pose(): x=%.2f, y=%.2f, z=%.2f, heading=%.2f(%.2f deg)",
        pose->translation.x, pose->translation.y, pose->translation.z,
        heading, heading / M_PI * 180);

    mikes_log(ML_DEBUG, str);
    printf("%s\n", str);
}


void get_t265_heading(t265_pose_type *pose, double *heading)
{
  rs2_quaternion *q = &(pose->rotation);
  *heading = M_PI-(atan2((2 * q->w * q->y - 2 * q->x * q->z),(q->x * q->x + q->y * q->y - q->w * q->w - q->z * q->z)));
}
