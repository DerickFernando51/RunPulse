#include "SensorManager.h"

#include "usbd_cdc_if.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ppg_dsp.h"
#include "imu_dsp.h"

#include "tasks.h"

#include <cstring>
#include <cstdio>


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
    // MAX30102
    if (!ppg_.init())
    {
        char msg[] = "MAX30102 INIT FAIL\r\n";

        CDC_Transmit_FS(
            (uint8_t*)msg,
            strlen(msg)
        );

        return false;
    }

    char msg1[] = "MAX30102 OK\r\n";

    CDC_Transmit_FS(
        (uint8_t*)msg1,
        strlen(msg1)
    );


    // KX126
    if (!imu_.init())
    {
        char msg[] = "KX126 INIT FAIL\r\n";

        CDC_Transmit_FS(
            (uint8_t*)msg,
            strlen(msg)
        );

        return false;
    }

    char msg2[] = "KX126 OK\r\n";

    CDC_Transmit_FS(
        (uint8_t*)msg2,
        strlen(msg2)
    );


    // MAX17048
    if (!battery_.init())
    {
        char msg[] = "MAX17048 INIT FAIL\r\n";

        CDC_Transmit_FS(
            (uint8_t*)msg,
            strlen(msg)
        );

        return false;
    }


    char msg3[] = "ALL SENSORS OK\r\n";

    CDC_Transmit_FS(
        (uint8_t*)msg3,
        strlen(msg3)
    );


    return true;
}


bool SensorManager::sampleFast(SensorFrame& frame)
{
    static uint8_t batteryCounter = 0;
    static uint8_t batterySOC = 0;
    static uint8_t printCounter = 0;

    AccelData accel;


    // =========================================================
    // KX126
    // =========================================================

    if (!imu_.readAcceleration(accel))
    {
        return false;
    }


    frame.ax = accel.x;
    frame.ay = accel.y;
    frame.az = accel.z;


    // IMU DSP
    IMU_PushSample(
        frame.ax,
        frame.ay,
        frame.az
    );


    // =========================================================
    // MAX30102
    // =========================================================

    if(ppg_.dataReady())
    {
        PPGData ppgData;

        while(ppg_.availableSamples() > 0)
        {
            if(!ppg_.getSample(ppgData))
            {
                break;
            }

            if(ppg_.fingerPresent(ppgData))
            {
                PPG_PushSample(
                    ppgData.red,
                    ppgData.ir
                );
            }
            else
            {
                PPG_SetFingerPresent(false);
            }
        }
    }


    // Start next DMA when enough FIFO data is available
    ppg_.startReadDMA();


    // =========================================================
    // MAX17048 - 1 Hz
    // =========================================================

    batteryCounter++;

    if (batteryCounter >= 100)
    {
        batteryCounter = 0;

        batterySOC = battery_.getStateOfCharge();
    }


    // =========================================================
    // PPG RESULT
    // =========================================================

    PPG_Result_t ppgResult = PPG_GetResult();


    // =========================================================
    // USB DEBUG / BLE - 1 Hz
    // =========================================================

    printCounter++;

    if (printCounter >= 100)
    {
        printCounter = 0;

        IMU_Result imuResult = IMU_GetResult();


        // USB debug
//        char usbBuf[64];
//
//        int len = snprintf(
//            usbBuf,
//            sizeof(usbBuf),
//            "Cadence: %u SPM\r\n",
//            imuResult.cadence
//        );
//
//        CDC_Transmit_FS(
//            (uint8_t*)usbBuf,
//            len
//        );


        // BLE queue
        BLE_Data_t bleData;

        bleData.cadence =
            imuResult.cadence;

        bleData.heartRate =
            ppgResult.heart_rate;

        bleData.spo2 =
            ppgResult.spo2;

        bleData.batterySOC =
            batterySOC;


        if (bleQueue != NULL)
        {
            osMessageQueuePut(
                bleQueue,
                &bleData,
                0,
                0
            );
        }
    }


    return true;
}
