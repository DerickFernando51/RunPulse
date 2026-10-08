#ifndef IMU_DSP_H
#define IMU_DSP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

void IMU_Process(void);          // called by DspTask when a buffer is full

IMU_Result IMU_GetResult(void);

#ifdef __cplusplus
}
#endif

#endif
