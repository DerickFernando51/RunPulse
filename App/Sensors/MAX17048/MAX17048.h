#ifndef MAX17048_H
#define MAX17048_H


#include "IBatteryMonitor.h"
#include "stm32wbxx_hal.h"



class MAX17048 : public IBatteryMonitor
{

public:

    explicit MAX17048(
        I2C_HandleTypeDef* hi2c
    );


    bool init() override;


    float getVoltage() override;


    float getStateOfCharge() override;



private:

    uint16_t readRegister(
        uint8_t reg
    );



private:

    I2C_HandleTypeDef* hi2c_;

};


#endif
