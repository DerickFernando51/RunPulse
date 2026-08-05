#include "Tasks.h"
#include "SensorManager.h"


extern SensorManager sensors;


void SensorTask(void *argument)
{

    TickType_t lastWake =
        xTaskGetTickCount();


    uint32_t batteryTimer = 0;


    sensors.init();


    while(1)
    {

        sensors.sampleFast();



        batteryTimer += 20;


        if(batteryTimer >= 1000)
        {
            batteryTimer = 0;

            sensors.sampleBattery();
        }



        vTaskDelayUntil(
            &lastWake,
            pdMS_TO_TICKS(20)
        );

    }

}
