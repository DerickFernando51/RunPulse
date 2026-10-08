#include "tasks.h"
#include "imu_dsp.h"
#include "ppg_dsp.h"

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

#define DSP_IMU_BIT  (1u << 0)
#define DSP_PPG_BIT  (1u << 1)

// Private handle
static osThreadId_t dspTaskHandle = NULL;

static void DspTask(void *argument)
{
    (void)argument;

    uint32_t bits = 0;

    for(;;)
    {
        // Sleep until a buffer is ready.
        // Clears all notification bits on exit so each event is handled once.
        xTaskNotifyWait(0, 0xFFFFFFFF, &bits, portMAX_DELAY);

        if(bits & DSP_IMU_BIT)
        {
            IMU_Process();
        }

        if(bits & DSP_PPG_BIT)
        {
            PPG_Process();
        }
    }
}

static const osThreadAttr_t dspTask_attributes =
{
    .name       = "DspTask",
    .stack_size = 1024 * 4,
    .priority   = osPriorityBelowNormal   // below SensorTask and BLETask
};

extern "C" void DspTask_Init(void)
{
    dspTaskHandle = osThreadNew(DspTask, NULL, &dspTask_attributes);
}

extern "C" void DSP_NotifyIMU(void)
{
    if(dspTaskHandle != NULL)
    {
        xTaskNotify((TaskHandle_t)dspTaskHandle, DSP_IMU_BIT, eSetBits);
    }
}

extern "C" void DSP_NotifyPPG(void)
{
    if(dspTaskHandle != NULL)
    {
        xTaskNotify((TaskHandle_t)dspTaskHandle, DSP_PPG_BIT, eSetBits);
    }
}
