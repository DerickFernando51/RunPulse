//#include "tasks.h"
//#include "SensorManager.h"
//#include "sensor_objects.h"
//
//#include "FreeRTOS.h"
//#include "task.h"
//
//extern SensorManager sensors;
//
//void PPGTask(void *argument)
//{
//    while(1)
//    {
//        sensors.processPPG();
//
//        /*
//         * MAX30102 is configured for 100 Hz.
//         * DMA completion is synchronized through
//         * the FreeRTOS semaphore inside the MAX30102 driver.
//         */
//        vTaskDelay(pdMS_TO_TICKS(100));
//    }
//}
//
//
