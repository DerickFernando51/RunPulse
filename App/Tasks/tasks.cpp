#include "tasks.h"
#include "cmsis_os.h"

static osThreadId_t sensorTaskHandle;
static osThreadId_t bleSeqTaskHandle;

osMessageQueueId_t cadenceBleQueue;

static const osThreadAttr_t sensorTask_attributes =
{
    .name = "SensorTask",
    .stack_size = 1024 * 8,
    .priority = osPriorityHigh
};

static const osThreadAttr_t bleSeqTask_attributes =
{
    .name = "BLESeqTask",
    .stack_size = 1024 * 4,
    .priority = osPriorityNormal
};

extern "C" void AppTasks_Init(void)
{
	cadenceBleQueue = osMessageQueueNew(
	        5,
	        sizeof(Cadence_BLE_Data_t),
	        NULL
	    );

    sensorTaskHandle =
        osThreadNew(
            SensorTask,
            NULL,
            &sensorTask_attributes
        );

    bleSeqTaskHandle =
        osThreadNew(
            BLESeqTask,
            NULL,
            &bleSeqTask_attributes
        );
}
