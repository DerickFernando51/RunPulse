#include "SensorManager.h"

#include "usbd_cdc_if.h"

#include "FreeRTOS.h"
#include "task.h"

#include <cstdio>
#include <cstring>


SensorManager::SensorManager(
        MAX30102& ppg,
        KX126& imu,
        MAX17048& battery)
:
ppg_(ppg),
imu_(imu),
battery_(battery)
{

}



bool SensorManager::init()
{

    return
        ppg_.init()
        &&
        imu_.init()
        &&
        battery_.init();

}

void SensorManager::sampleFast()
{
    AccelData accel;


    if(!imu_.readAcceleration(accel))
    {
        char msg[]="KX126 READ FAIL\r\n";

        CDC_Transmit_FS(
            (uint8_t*)msg,
            strlen(msg)
        );

        return;
    }


    char usbBuf[64];


    int len = snprintf(
        usbBuf,
        sizeof(usbBuf),
        "ACC %.4f %.4f %.4f\r\n",
        accel.x,
        accel.y,
        accel.z
    );

    CDC_Transmit_FS(
        (uint8_t*)usbBuf,
        len
    );
}

//void SensorManager::sampleFast()
//{
//    static uint32_t printCounter = 0;
//
//    AccelData accel;
//
//
//    if(!imu_.readAcceleration(accel))
//    {
//        char msg[] = "KX126 READ FAIL\r\n";
//
//        CDC_Transmit_FS(
//            (uint8_t*)msg,
//            strlen(msg)
//        );
//
//        return;
//    }
//
//
//    printCounter++;
//
//
//    if(printCounter >= 50)
//    {
//        printCounter = 0;
//
//
//        char usbBuf[64];
//
//
//        int len = snprintf(
//                usbBuf,
//                sizeof(usbBuf),
//                "ACC %.2f %.2f %.2f\r\n",
//                accel.x,
//                accel.y,
//                accel.z
//        );
//
//
//        if(len > 0)
//        {
//            CDC_Transmit_FS(
//                (uint8_t*)usbBuf,
//                len
//            );
//        }
//    }
//}
