#include "SensorManager.h"

//#include "usbd_cdc_if.h"


#include "ppg_dsp.h"
#include "imu_dsp.h"

#include "tasks.h"



SensorManager::SensorManager(
    IPPGSensor& ppg,
    IIMUSensor& imu,
    IBatterySensor& battery,
    IBleSink& bleSink)
    :
    ppg_(ppg),
    imu_(imu),
    battery_(battery),
    bleSink_(bleSink)
{
}


bool SensorManager::init()
{
    // MAX30102
    if (!ppg_.init())
    {
//        char msg[] = "MAX30102 INIT FAIL\r\n";
//
//        CDC_Transmit_FS(
//            (uint8_t*)msg,
//            strlen(msg)
//        );

        return false;
    }

//    char msg1[] = "MAX30102 OK\r\n";

//    CDC_Transmit_FS(
//        (uint8_t*)msg1,
//        strlen(msg1)
//    );


    // KX126
    if (!imu_.init())
    {
//        char msg[] = "KX126 INIT FAIL\r\n";
//
//        CDC_Transmit_FS(
//            (uint8_t*)msg,
//            strlen(msg)
//        );

        return false;
    }

//    char msg2[] = "KX126 OK\r\n";

//    CDC_Transmit_FS(
//        (uint8_t*)msg2,
//        strlen(msg2)
//    );


    // MAX17048
    if (!battery_.init())
    {
//        char msg[] = "MAX17048 INIT FAIL\r\n";
//
//        CDC_Transmit_FS(
//            (uint8_t*)msg,
//            strlen(msg)
//        );

        return false;
    }


//    char msg3[] = "ALL SENSORS OK\r\n";
//
//    CDC_Transmit_FS(
//        (uint8_t*)msg3,
//        strlen(msg3)
//    );


    return true;
}


bool SensorManager::sampleFast(SensorFrame& frame)
{
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

    IMU_PushSample(frame.ax, frame.ay, frame.az);

    // =========================================================
    // MAX30102
    // =========================================================
    if (ppg_.dataReady())
    {
        PPGData ppgData;

        while (ppg_.availableSamples() > 0)
        {
            if (!ppg_.getSample(ppgData))
            {
                break;
            }

            if (ppg_.fingerPresent(ppgData))
            {
                PPG_PushSample(ppgData.red, ppgData.ir);
            }
            else
            {
                PPG_SetFingerPresent(false);
            }
        }
    }

    ppg_.startReadDMA();

    // =========================================================
    // MAX17048 - 1 Hz
    // =========================================================
    batteryCounter_++;

    if (batteryCounter_ >= 100)
    {
        batteryCounter_ = 0;
        batterySOC_ = battery_.getStateOfCharge();
    }

    // =========================================================
    // PPG RESULT
    // =========================================================
    PPG_Result_t ppgResult = PPG_GetResult();

    // =========================================================
    // USB DEBUG / BLE - 1 Hz
    // =========================================================
    printCounter_++;
//
//    if (printCounter_ >= 100)
//    {
//        printCounter_ = 0;
//
//        IMU_Result imuResult = IMU_GetResult();
//
//        BLE_Data_t bleData;
//        bleData.cadence     = imuResult.cadence;
//        bleData.heartRate   = ppgResult.heart_rate;
//        bleData.spo2        = ppgResult.spo2;
//        bleData.batterySOC  = batterySOC_;
//
//        bleSink_.publish(bleData);
//    }

    return true;
}
