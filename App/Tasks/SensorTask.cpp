#include "tasks.h"
#include "SensorManager.h"
#include "sensor_objects.h"

#include "FreeRTOS.h"
#include "task.h"

#include "string.h"

extern "C"
{
#include "usbd_cdc_if.h"
}


extern SensorManager sensors;

void SensorTask(void *argument)
{
	Sensors_Init();
	vTaskDelay(pdMS_TO_TICKS(3000));

    char msg[] = "TASK START\r\n";

       CDC_Transmit_FS(
           (uint8_t*)msg,
           strlen(msg)
       );

       vTaskDelay(pdMS_TO_TICKS(3000));


       if(sensors.init())
       {
           char msg[]="SENSORS INIT OK\r\n";

           CDC_Transmit_FS(
               (uint8_t*)msg,
               strlen(msg)
           );
       }
       else
       {
           char msg[]="SENSORS INIT FAIL\r\n";

           CDC_Transmit_FS(
               (uint8_t*)msg,
               strlen(msg)
           );
       }

    TickType_t lastWake = xTaskGetTickCount();


    while(1)
    {

    	SensorFrame frame = {};

    	if(sensors.sampleFast(frame))
    	{
    	    // frame contains:
    	    // frame.ax
    	    // frame.ay
    	    // frame.az
    	    // frame.ir
    	    // frame.red
    	}

        vTaskDelayUntil(
            &lastWake,
            pdMS_TO_TICKS(20)
        );

    }
}
