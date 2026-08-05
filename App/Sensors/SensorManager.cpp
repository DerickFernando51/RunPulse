#include "SensorManager.h"


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

    // IMU 50Hz

    AccelData accel;

    if(imu_.readAcceleration(accel))
    {
        // store/send
    }



    // PPG DMA

    if(!ppg_.dataReady())
    {
        ppg_.startReadDMA();
    }
    else
    {
        PPGData ppgData;

        if(ppg_.getSample(ppgData))
        {
            // store/send
        }
    }

}



void SensorManager::sampleBattery()
{

    float voltage =
        battery_.getVoltage();


    float soc =
        battery_.getStateOfCharge();


    // store/send battery info

}
