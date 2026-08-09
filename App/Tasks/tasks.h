#pragma once

#include <stdint.h>
#include "cmsis_os2.h"

typedef struct
{
    uint16_t cadence;
} Cadence_BLE_Data_t;

extern osMessageQueueId_t cadenceBleQueue;

#ifdef __cplusplus
extern "C" {
#endif

void AppTasks_Init(void);
void SensorTask(void *argument);
void BLESeqTask(void *argument);
void BLETask(void *argument);

#ifdef __cplusplus
}
#endif
