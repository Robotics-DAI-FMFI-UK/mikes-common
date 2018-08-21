#include <XnOpenNI.h>
#include <XnLog.h>
#include <XnCppWrapper.h>
#include <XnFPSCalculator.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "xtion.h"

extern "C" {

#include "../../../bites/mikes.h"
#include "../../passive/mikes_logs.h"
#include "core/config_mikes.h"

}

static pthread_mutex_t xtion_lock;
static xtion_pixel *scaled;
static int width, height;
static int resolution_x, resolution_y;
static double qx, qy;

static int online;

void *xtion_thread(void *args)
{
    XnStatus nRetVal = XN_STATUS_OK;
    xn::Context context;
    xtion_pixel *scaled_image = (xtion_pixel *) malloc(width * height * sizeof(xtion_pixel));

    // Initialize context object
    //nRetVal = context.Init();

        xn::ScriptNode scriptNode;
        xn::EnumerationErrors errors;

        mikes_log_str(ML_INFO, "xtion reading config from:", mikes_config.xtion_samples_config);
        nRetVal = context.InitFromXmlFile(mikes_config.xtion_samples_config, scriptNode, &errors);

        if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
        {
                XnChar strError[1024];
                errors.ToString(strError, 1024);
                mikes_log_str(ML_ERR, "xtion reading config: ", strError);
                threads_running_add(-1);
                return 0;
        }
        else if (nRetVal != XN_STATUS_OK)
        {
                mikes_log_str(ML_ERR, "xtion open config failed: ", xnGetStatusString(nRetVal));
                threads_running_add(-1);
                return 0;
        }

    // Create a DepthGenerator node
    xn::DepthGenerator depth;
    nRetVal = depth.Create(context);

    if (nRetVal != XN_STATUS_OK)
    {
        mikes_log(ML_ERR, "xtion create depth generator failed");
        return 0;
    }

    XnMapOutputMode mapMode;
    if ((width > 320) || (height > 240)) 
    {
      resolution_x = 640;
      resolution_y = 480;
    }
    else
    {
      resolution_x = 320;
      resolution_y = 240;
    }

    mapMode.nXRes = resolution_x;
    mapMode.nYRes = resolution_y;
    qx = resolution_x / width;
    qy = resolution_y / height;
    mapMode.nFPS = 30;
    nRetVal = depth.SetMapOutputMode(mapMode);

    // Make it start generating data
    nRetVal = context.StartGeneratingAll();

    if (nRetVal != XN_STATUS_OK)
    {
        mikes_log(ML_ERR, "xtion start generating failed");
        threads_running_add(-1);
        return 0;
    }

    // Main loop
    while (program_runs)
    {
        // Wait for new data to be available
        nRetVal = context.WaitOneUpdateAll(depth);
        if (nRetVal != XN_STATUS_OK)
        {
            mikes_log_str(ML_ERR, "xtion failed updating data: ", xnGetStatusString(nRetVal));
            continue;
        }
        // Take current depth map
        const XnDepthPixel* pDepthMap = depth.GetDepthMap();

        for (int row = 0; row < height; row++)
          for (int col = 0; col < width; col++)
          {
             int r1 = (int)(row * qy + 0.5);
             int r2 = (int)((row + 1) * qy - 0.5);
             int c1 = resolution_x - (int)(col * qx + 0.5) - 1;
             int c2 = resolution_x - (int)((col + 1) * qx - 0.5) - 1;
             int minD = 6000;

             for (int r = r1; r <= r2; r++)
               for (int c = c1; c >= c2; c--)
               {
                 int dep = pDepthMap[r * resolution_x + c];
                 if ((dep != 0) && (dep < minD)) minD = dep;
               }

             scaled_image[row * width + col] = minD;
          }

    	pthread_mutex_lock(&xtion_lock);
    	memcpy(scaled, scaled_image, sizeof(xtion_pixel) * width * height);
    	pthread_mutex_unlock(&xtion_lock);

    }
    // Clean-up
    xnContextRelease(context.GetUnderlyingObject());
    mikes_log(ML_INFO, "xtion quits.");
    threads_running_add(-1);
    return 0;
}

void get_xtion_data(xtion_pixel* buffer)
{
    if (!online) return;

    pthread_mutex_lock(&xtion_lock);
    memcpy(buffer, scaled, sizeof(xtion_pixel) * width * height);
    pthread_mutex_unlock(&xtion_lock);
}

void init_xtion(int data_width, int data_height)
{
    if (!mikes_config.use_xtion)
    {
        mikes_log(ML_INFO, "Asus xtion supressed by config.");
        online = 0;
        return;
    }
    online = 1;

    pthread_t t;
    pthread_mutex_init(&xtion_lock, 0);
    width = data_width;
    height = data_height;
    scaled = (xtion_pixel *) malloc(width * height * sizeof(xtion_pixel));
    if (pthread_create(&t, 0, xtion_thread, 0) != 0)
    {
      perror("mikes:xtion");
      mikes_log(ML_ERR, "creating thread for asus xtion");
    }
    else threads_running_add(1);
}

