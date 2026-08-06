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

    if(!ppg_.init())
    {
        char msg[]="MAX30102 INIT FAIL\r\n";
        CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
        return false;
    }

    char msg1[]="MAX30102 OK\r\n";
    CDC_Transmit_FS((uint8_t*)msg1, strlen(msg1));


    if(!imu_.init())
    {
        char msg[]="KX126 INIT FAIL\r\n";
        CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
        return false;
    }

    char msg2[]="KX126 OK\r\n";
    CDC_Transmit_FS((uint8_t*)msg2, strlen(msg2));


    if(!battery_.init())
    {
        char msg[]="MAX17048 INIT FAIL\r\n";
        CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
        return false;
    }


    char msg3[]="ALL SENSORS OK\r\n";
    CDC_Transmit_FS((uint8_t*)msg3, strlen(msg3));

    return true;
}


bool SensorManager::sampleFast(SensorFrame& frame)
{
    AccelData accel;


    // KX126
    if(!imu_.readAcceleration(accel))
    {
        char msg[] = "KX126 READ FAIL\r\n";

        CDC_Transmit_FS(
            (uint8_t*)msg,
            strlen(msg)
        );

        return false;
    }


    frame.ax = accel.x;
    frame.ay = accel.y;
    frame.az = accel.z;



    // MAX30102 DMA

    if(!ppg_.startReadDMA())
    {
        frame.ir = 0;
        frame.red = 0;
        return false;
    }

    PPGData ppg;

    if(ppg_.getSample(ppg))
    {
        frame.ir  = ppg.ir;
        frame.red = ppg.red;
    }
    else
    {
        frame.ir = 0;
        frame.red = 0;
    }


    // USB DEBUG OUTPUT

    char usbBuf[128];


    int len = snprintf(
        usbBuf,
        sizeof(usbBuf),
        "ACC %.3f %.3f %.3f | IR %lu RED %lu\r\n",
        frame.ax,
        frame.ay,
        frame.az,
        frame.ir,
        frame.red
    );


    CDC_Transmit_FS(
        (uint8_t*)usbBuf,
        len
    );


    return true;
}


//bool SensorManager::sampleBattery(SensorFrame& frame)
//{
//    return battery_.read(frame.battery);
//}
