#ifndef SENSORFRAME_H
#define SENSORFRAME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float ax;
    float ay;
    float az;

    uint32_t ir;
    uint32_t red;

    float batteryVoltage;
    float batterySOC;

} SensorFrame;

#ifdef __cplusplus
}
#endif

#endif
