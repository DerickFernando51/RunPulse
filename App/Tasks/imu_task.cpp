//#include "tasks.h"
//#include "SensorManager.h"
//#include "sensor_objects.h"
//
//#include "FreeRTOS.h"
//#include "task.h"
//
//extern SensorManager sensors;
//
//void IMUTask(void *argument)
//{
//    SensorFrame frame = {};
//
//    TickType_t lastWake =
//        xTaskGetTickCount();
//
//    while(1)
//    {
//        sensors.sampleIMU(frame);
//
//        vTaskDelayUntil(
//            &lastWake,
//            pdMS_TO_TICKS(10)
//        );
//    }
//}
