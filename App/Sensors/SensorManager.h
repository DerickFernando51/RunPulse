#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "MAX30102.h"
#include "KX126.h"
#include "MAX17048.h"


class SensorManager
{

public:

    SensorManager(
        MAX30102& ppg,
        KX126& imu,
        MAX17048& battery
    );


    bool init();


    void sampleFast();

    void sampleBattery();


private:

    MAX30102& ppg_;
    KX126& imu_;
    MAX17048& battery_;

};


#endif
