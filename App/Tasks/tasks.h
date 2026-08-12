#pragma once

#include <stdint.h>
#include "cmsis_os2.h"

typedef struct //Custom BLE Frame structure - 7 bytes
{
    uint16_t cadence;
    uint16_t heartRate;
    uint8_t  spo2;
    uint8_t  batterySOC;
} BLE_Data_t;

extern osMessageQueueId_t bleQueue;

#ifdef __cplusplus
extern "C" {
#endif

void AppTasks_Init(void);
void SensorTask(void *argument);
void BLETask(void *argument);

#ifdef __cplusplus
}
#endif
