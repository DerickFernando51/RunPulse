#include "imu_dsp.h"

#include <math.h>
#include <string.h>
#include <stdbool.h>

#define IMU_BUF_LEN                     500
#define SAMPLE_RATE_HZ                  100.0f

#define MIN_CADENCE_SPM                 40.0f
#define MAX_CADENCE_SPM                 200.0f

#define MIN_PEAK_DISTANCE_SAMPLES       30
#define MIN_PEAKS                       3

#define MIN_MOVEMENT_RMS                0.02f
#define PEAK_THRESHOLD_FRACTION         0.5f
#define MAX_INTERVAL_DEVIATION          0.50f

#define CADENCE_EMA_ALPHA               0.8f // % of newest reading used for cadence calculation


// Internal data

static float accelBuffer[IMU_BUF_LEN];
static uint16_t bufferIndex = 0;

static IMU_Result result = {0};
static float cadenceFiltered = 0.0f;


// Calculate acceleration magnitude

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


// Calculate mean

static float mean(
    float *data,
    uint16_t len)
{
    float sum = 0.0f;

    for(uint16_t i = 0; i < len; i++)
    {
        sum += data[i];
    }

    return sum / (float)len;
}


// Smooth acceleration signal

static void smooth(
    float *input,
    float *output,
    uint16_t len)
{
    if(len == 0)
        return;

    if(len == 1)
    {
        output[0] = input[0];
        return;
    }

    output[0] = input[0];

    for(uint16_t i = 1; i < len - 1; i++)
    {
        output[i] =
            (
                input[i - 1] +
                input[i] +
                input[i + 1]
            ) / 3.0f;
    }

    output[len - 1] = input[len - 1];
}


// Calculate movement RMS

static float rmsAC(
    float *data,
    uint16_t len,
    float dc)
{
    float sum = 0.0f;

    for(uint16_t i = 0; i < len; i++)
    {
        float ac = data[i] - dc;

        sum += ac * ac;
    }

    return sqrtf(
        sum / (float)len
    );
}


// Check peak consistency

static bool intervalsAreConsistent(
    uint16_t *peaks,
    uint16_t count)
{
    if(count < 3)
        return true;

    float averageInterval = 0.0f;

    uint16_t intervals = count - 1;

    for(uint16_t i = 1; i < count; i++)
    {
        averageInterval +=
            (float)(
                peaks[i] -
                peaks[i - 1]
            );
    }

    averageInterval /=
        (float)intervals;

    if(averageInterval <= 0.0f)
        return false;

    for(uint16_t i = 1; i < count; i++)
    {
        float interval =
            (float)(
                peaks[i] -
                peaks[i - 1]
            );

        float difference =
            fabsf(
                interval -
                averageInterval
            );

        float relativeDifference =
            difference /
            averageInterval;

        if(
            relativeDifference >
            MAX_INTERVAL_DEVIATION
        )
        {
            return false;
        }
    }

    return true;
}


// Detect peaks and calculate cadence

static float calculateCadence(
    float *accel,
    uint16_t len,
    float dc,
    float movementRMS)
{
    uint16_t peaks[30];
    uint16_t peakCount = 0;

    float threshold =
        movementRMS *
        PEAK_THRESHOLD_FRACTION;

    if(threshold < 0.03f)
    {
        threshold = 0.03f;
    }

    for(uint16_t i = 1;
        i < len - 1;
        i++)
    {
        float prev =
            accel[i - 1] - dc;

        float curr =
            accel[i] - dc;

        float next =
            accel[i + 1] - dc;

        bool isPeak =
            curr > prev &&
            curr > next &&
            curr > threshold;

        if(!isPeak)
            continue;

        if(peakCount == 0)
        {
            peaks[peakCount++] = i;
        }
        else
        {
            uint16_t distance =
                i -
                peaks[peakCount - 1];

            if(
                distance >=
                MIN_PEAK_DISTANCE_SAMPLES
            )
            {
                if(peakCount < 30)
                {
                    peaks[peakCount++] = i;
                }
            }
        }
    }

    if(peakCount < MIN_PEAKS)
    {
        return 0.0f;
    }

    if(
        !intervalsAreConsistent(
            peaks,
            peakCount
        )
    )
    {
        return 0.0f;
    }

    float totalDistance = 0.0f;

    uint16_t intervals =
        peakCount - 1;

    for(uint16_t i = 1;
        i < peakCount;
        i++)
    {
        totalDistance +=
            (float)(
                peaks[i] -
                peaks[i - 1]
            );
    }

    float averageDistance =
        totalDistance /
        (float)intervals;

    if(averageDistance <= 0.0f)
    {
        return 0.0f;
    }

    float stepTime =
        averageDistance /
        SAMPLE_RATE_HZ;

    if(stepTime <= 0.0f)
    {
        return 0.0f;
    }

    float cadence =
        60.0f /
        stepTime;

    if(
        cadence < MIN_CADENCE_SPM ||
        cadence > MAX_CADENCE_SPM
    )
    {
        return 0.0f;
    }

    return cadence;
}


// Process cadence

static void processCadence()
{
    float accel[IMU_BUF_LEN];

    smooth(
        accelBuffer,
        accel,
        IMU_BUF_LEN
    );

    float dc =
        mean(
            accel,
            IMU_BUF_LEN
        );

    float movementRMS =
        rmsAC(
            accel,
            IMU_BUF_LEN,
            dc
        );

    // Detect stationary state

    if(
        movementRMS <
        MIN_MOVEMENT_RMS
    )
    {
        result.cadence = 0;
        result.stepCount = 0;
        result.valid = 0;

        cadenceFiltered = 0.0f;

        return;
    }

    float cadence =
        calculateCadence(
            accel,
            IMU_BUF_LEN,
            dc,
            movementRMS
        );

    if(cadence <= 0.0f)
    {
        result.valid = 0;
        return;
    }

    // Smooth cadence

    if(cadenceFiltered == 0.0f)
    {
        cadenceFiltered = cadence;
    }
    else
    {
        cadenceFiltered =
            CADENCE_EMA_ALPHA * cadence +
            (1.0f - CADENCE_EMA_ALPHA)
                * cadenceFiltered;
    }

    result.cadence =
        (uint16_t)cadenceFiltered;

    result.valid = 1;
}


// Initialise DSP

void IMU_Init(void)
{
    memset(
        accelBuffer,
        0,
        sizeof(accelBuffer)
    );

    bufferIndex = 0;

    cadenceFiltered = 0.0f;

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

    if(bufferIndex >= IMU_BUF_LEN)
    {
        processCadence();

        bufferIndex = 0;
    }
}


// Get cadence result

IMU_Result IMU_GetResult(void)
{
    return result;
}
