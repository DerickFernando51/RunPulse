#ifndef IMU_DSP_H
#define IMU_DSP_H

#include <stdint.h>

struct IMU_Result
{
    uint16_t cadence;
    uint32_t stepCount;
    uint8_t valid;
};

void IMU_Init(void);

void IMU_PushSample(
    float ax,
    float ay,
    float az
);

IMU_Result IMU_GetResult(void);

#endif
