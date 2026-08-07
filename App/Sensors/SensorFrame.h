#ifndef SENSORFRAME_H
#define SENSORFRAME_H

#include <stdint.h>

struct SensorFrame
{
    float ax;
    float ay;
    float az;

    uint32_t ir;
    uint32_t red;

    float batteryVoltage;
    float batterySOC;
};

#endif
