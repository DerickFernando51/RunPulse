#ifndef KX126_HPP
#define KX126_HPP


#include "IIMUSensor.h"
#include "stm32wbxx_hal.h"


#define KX126_CS_PORT GPIOA
#define KX126_CS_PIN  GPIO_PIN_4


#define KX126_READ 0x80


#define KX126_WHO_AM_I_REG 0x11


#define KX126_CNTL1  0x1A
#define KX126_ODCNTL 0x1F


#define KX126_XOUT_L 0x08



class KX126 : public IIMUSensor
{

public:

    explicit KX126(
        SPI_HandleTypeDef* hspi
    );


    bool init() override;


    bool readAcceleration(
        AccelData& data
    ) override;



private:

    bool writeRegister(
        uint8_t reg,
        uint8_t value
    );


    bool readRegister(
        uint8_t reg,
        uint8_t* data,
        uint16_t length
    );


private:

    SPI_HandleTypeDef* hspi_;

};


#endif
