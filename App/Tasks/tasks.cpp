#include "tasks.h"
#include "cmsis_os.h"

static osThreadId_t sensorTaskHandle;
static osThreadId_t bleTaskHandle;

osMessageQueueId_t bleQueue;

static const osThreadAttr_t sensorTask_attributes =
{
    .name = "SensorTask",
    .stack_size = 1024 * 8,
    .priority = osPriorityHigh
};

static const osThreadAttr_t bleTask_attributes =
{
    .name = "BLETask",
    .stack_size = 1024 * 4,
    .priority = osPriorityNormal
};

extern "C" void AppTasks_Init(void)
{
	bleQueue = osMessageQueueNew(
	        5,
	        sizeof(BLE_Data_t),
	        NULL
	    );

    sensorTaskHandle =
        osThreadNew(
            SensorTask,
            NULL,
            &sensorTask_attributes
        );

    //bleTaskHandle =
        osThreadNew(
            BLETask,
            NULL,
            &bleTask_attributes
        );
}
