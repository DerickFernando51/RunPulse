#include "tasks.h"
#include "cmsis_os.h"


extern void SensorTask(void *argument);


static osThreadId_t sensorTaskHandle;


static const osThreadAttr_t sensorTask_attributes =
{
    .name = "SensorTask",
    .stack_size = 1024 * 8,
    .priority = osPriorityHigh
};



extern "C" void AppTasks_Init(void)
{
    sensorTaskHandle =
        osThreadNew(
            SensorTask,
            NULL,
            &sensorTask_attributes
        );
}
