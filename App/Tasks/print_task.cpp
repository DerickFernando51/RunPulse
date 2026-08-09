//#include "tasks.h"
//#include "SensorManager.h"
//#include "sensor_objects.h"
//
//#include "FreeRTOS.h"
//#include "task.h"
//
//#include "ppg_dsp.h"
//#include "imu_dsp.h"
//
//#include <stdio.h>
//#include <string.h>
//
//extern "C"
//{
//#include "usbd_cdc_if.h"
//}
//
//extern SensorManager sensors;
//
//extern SensorFrame batteryFrame;
//
//void PrintTask(void *argument)
//{
//    while(1)
//    {
//        PPG_Result_t ppgResult =
//            PPG_GetResult();
//
//        IMU_Result imuResult =
//            IMU_GetResult();
//
//        char usbBuf[160];
//
//        int len;
//
//        if(ppgResult.valid && imuResult.valid)
//        {
//            len = snprintf(
//                usbBuf,
//                sizeof(usbBuf),
//
//                "HR %u BPM | SpO2 %u%% | CAD %u SPM | BAT %.2fV %.0f%%\r\n",
//
//                ppgResult.heart_rate,
//                ppgResult.spo2,
//                imuResult.cadence,
//                batteryFrame.batteryVoltage,
//                batteryFrame.batterySOC
//            );
//        }
//        else if(ppgResult.valid)
//        {
//            len = snprintf(
//                usbBuf,
//                sizeof(usbBuf),
//
//                "HR %u BPM | SpO2 %u%% | CAD -- | BAT %.2fV %.0f%%\r\n",
//
//                ppgResult.heart_rate,
//                ppgResult.spo2,
//                batteryFrame.batteryVoltage,
//                batteryFrame.batterySOC
//            );
//        }
//        else if(imuResult.valid)
//        {
//            len = snprintf(
//                usbBuf,
//                sizeof(usbBuf),
//
//                "HR -- | SpO2 -- | CAD %u SPM | BAT %.2fV %.0f%%\r\n",
//
//                imuResult.cadence,
//                batteryFrame.batteryVoltage,
//                batteryFrame.batterySOC
//            );
//        }
//        else
//        {
//            len = snprintf(
//                usbBuf,
//                sizeof(usbBuf),
//
//                "HR -- | SpO2 -- | CAD -- | BAT %.2fV %.0f%%\r\n",
//
//                batteryFrame.batteryVoltage,
//                batteryFrame.batterySOC
//            );
//        }
//
//        CDC_Transmit_FS(
//            (uint8_t*)usbBuf,
//            len
//        );
//
//        vTaskDelay(
//            pdMS_TO_TICKS(1000)
//        );
//    }
//}
//
//
////void PrintTask(void *argument)
////{
////    const char msg[] = "PRINT TASK STARTED\r\n";
////
////    while (1)
////    {
////        uint8_t status = CDC_Transmit_FS(
////            (uint8_t *)msg,
////            strlen(msg)
////        );
////
////        // Put breakpoint here
////        // Check status
////
////        vTaskDelay(pdMS_TO_TICKS(1000));
////    }
////}
