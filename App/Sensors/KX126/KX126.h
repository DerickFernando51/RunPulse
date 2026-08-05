#ifndef KX126_H
#define KX126_H

#include "IIMUSensor.h"
#include "stm32wbxx_hal.h"


class KX126 : public IIMUSensor
{
public:

    explicit KX126(SPI_HandleTypeDef* hspi);


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
