#include "imu_dsp.h"

#include <math.h>
#include <string.h>


#define SAMPLE_RATE_HZ              100.0f

#define BUFFER_LEN                  500

#define MIN_STEP_DISTANCE_SAMPLES  25

#define STEP_THRESHOLD              0.12f


static float accelBuffer[BUFFER_LEN];

static uint16_t bufferIndex = 0;

static IMU_Result result = {0};


// Acceleration magnitude

static float magnitude(
    float ax,
    float ay,
    float az)
{
    return sqrtf(
        ax * ax +
        ay * ay +
        az * az
    );
}


// Mean

static float mean(
    float *data,
    uint16_t len)
{
    float sum = 0.0f;

    for(uint16_t i = 0; i < len; i++)
    {
        sum += data[i];
    }

    return sum / len;
}


// Process cadence

static void processCadence()
{
    float dc = mean(
        accelBuffer,
        BUFFER_LEN
    );


    uint16_t peaks[30];

    uint16_t peakCount = 0;


    for(uint16_t i = 2;
        i < BUFFER_LEN - 2;
        i++)
    {
        float prev2 =
            accelBuffer[i - 2] - dc;

        float prev1 =
            accelBuffer[i - 1] - dc;

        float curr =
            accelBuffer[i] - dc;

        float next1 =
            accelBuffer[i + 1] - dc;

        float next2 =
            accelBuffer[i + 2] - dc;


        bool isPeak =
            curr > prev2 &&
            curr > prev1 &&
            curr > next1 &&
            curr > next2 &&
            curr > STEP_THRESHOLD;


        if(!isPeak)
            continue;


        if(peakCount == 0)
        {
            peaks[peakCount++] = i;
        }
        else
        {
            uint16_t distance =
                i - peaks[peakCount - 1];


            if(distance >= MIN_STEP_DISTANCE_SAMPLES)
            {
                if(peakCount < 30)
                {
                    peaks[peakCount++] = i;
                }
            }
        }
    }


    if(peakCount < 2)
    {
        result.valid = 0;
        return;
    }


    float totalDistance = 0.0f;

    uint16_t intervals = 0;


    for(uint16_t i = 1;
        i < peakCount;
        i++)
    {
        totalDistance +=
            peaks[i] - peaks[i - 1];

        intervals++;
    }


    float averageDistance =
        totalDistance / intervals;


    float stepTime =
        averageDistance / SAMPLE_RATE_HZ;


    if(stepTime <= 0.0f)
    {
        result.valid = 0;
        return;
    }


    float cadence =
        60.0f / stepTime;


    if(cadence < 40.0f ||
       cadence > 240.0f)
    {
        result.valid = 0;
        return;
    }


    result.cadence =
        (uint16_t)cadence;

    result.stepCount =
        peakCount;

    result.valid = 1;
}


// Initialise IMU DSP


void IMU_Init(void)
{
    memset(
        accelBuffer,
        0,
        sizeof(accelBuffer)
    );

    bufferIndex = 0;

    memset(
        &result,
        0,
        sizeof(result)
    );
}


// Push IMU sample

void IMU_PushSample(
    float ax,
    float ay,
    float az)
{
    float accMag =
        magnitude(
            ax,
            ay,
            az
        );


    accelBuffer[bufferIndex] =
        accMag;


    bufferIndex++;


    if(bufferIndex >= BUFFER_LEN)
    {
        processCadence();

        bufferIndex = 0;
    }
}


// Get result

IMU_Result IMU_GetResult(void)
{
    return result;
}
