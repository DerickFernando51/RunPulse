#ifndef MAX30102_H
#define MAX30102_H


#include "IPPGSensor.h"
#include "stm32wbxx_hal.h"



class MAX30102 : public IPPGSensor
{

public:


    explicit MAX30102(
        I2C_HandleTypeDef* hi2c
    );


    bool init() override;


    bool startReadDMA() override;


    bool dataReady() const override;


    bool getSample(
        PPGData& data
    ) override;


    bool fingerPresent(
        const PPGData& data
    ) const override;



    void rxCompleteCallback(
        I2C_HandleTypeDef* hi2c
    );



private:


    bool writeRegister(
        uint8_t reg,
        uint8_t value
    );


private:


    I2C_HandleTypeDef* hi2c_;


    volatile bool dmaComplete_;


    uint8_t dmaBuffer_[6];

};


#endif
