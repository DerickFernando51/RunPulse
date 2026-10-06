#include "tasks.h"
#include "SensorManager.h"
#include "sensor_objects.h"

#include "stm32wbxx.h"

#include "FreeRTOS.h"
#include "task.h"

extern SensorManager sensors;

// Live Expressions
volatile uint32_t g_maxWorkUs    = 0;   // worst work time in last second
volatile uint32_t g_minPeriodUs  = 0;   // shortest loop period in last second
volatile uint32_t g_maxPeriodUs  = 0;   // longest loop period in last second
volatile uint32_t g_overruns     = 0;   // total missed deadlines
volatile uint32_t g_worstWorkUs  = 0;   // all-time worst work time

void SensorTask(void *argument)
{
    (void)argument;

    Sensors_Init();
    vTaskDelay(pdMS_TO_TICKS(3000));

    vTaskDelay(pdMS_TO_TICKS(3000));

    sensors.init();

    // Enable the DWT cycle counter
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    TickType_t lastWake = xTaskGetTickCount();

    const uint32_t cyclesPerUs = SystemCoreClock / 1000000;

    uint32_t lastStart  = 0;
    uint32_t minPeriod  = 0xFFFFFFFF;
    uint32_t maxPeriod  = 0;
    uint32_t maxWork    = 0;
    uint32_t worstWork  = 0;
    uint32_t overruns   = 0;
    uint32_t loopCount  = 0;

    while(1)
    {
        uint32_t start = DWT->CYCCNT;

        SensorFrame frame = {};
        sensors.sampleFast(frame);

        uint32_t work = DWT->CYCCNT - start;
        if(work > maxWork)   maxWork   = work;
        if(work > worstWork) worstWork = work;

        if(lastStart != 0)
        {
            uint32_t period = start - lastStart;
            if(period < minPeriod) minPeriod = period;
            if(period > maxPeriod) maxPeriod = period;
        }
        lastStart = start;

        // Detect missed deadlines
        TickType_t expectedWake = lastWake + pdMS_TO_TICKS(10);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));

        if((TickType_t)(xTaskGetTickCount() - expectedWake) > 1)
        {
            overruns++;
        }

        // Publish once per second
        if(++loopCount >= 100)
        {
            loopCount = 0;

            g_maxWorkUs   = maxWork   / cyclesPerUs;
            g_minPeriodUs = minPeriod / cyclesPerUs;
            g_maxPeriodUs = maxPeriod / cyclesPerUs;
            g_overruns    = overruns;
            g_worstWorkUs = worstWork / cyclesPerUs;

            // reset the one-second window
            minPeriod = 0xFFFFFFFF;
            maxPeriod = 0;
            maxWork   = 0;
        }
    }
}
