#include "KX126.h"


#define KX126_WHO_AM_I_REG     0x13
#define KX126_DATA_START_REG   0x06


KX126::KX126(SPI_HandleTypeDef* hspi)
    :
    hspi_(hspi)
{

}


bool KX126::init()
{
    uint8_t id;

    if(!readRegister(
            KX126_WHO_AM_I_REG,
            &id,
            1))
    {
        return false;
    }


    // Configure KX126 registers here


    return true;
}



bool KX126::readAcceleration(
        AccelData& data)
{
    uint8_t buffer[6];


    if(!readRegister(
            KX126_DATA_START_REG,
            buffer,
            6))
    {
        return false;
    }


    int16_t x =
        (buffer[1] << 8) |
         buffer[0];


    int16_t y =
        (buffer[3] << 8) |
         buffer[2];


    int16_t z =
        (buffer[5] << 8) |
         buffer[4];


    data.x = x * 0.000244f;
    data.y = y * 0.000244f;
    data.z = z * 0.000244f;


    return true;
}



bool KX126::writeRegister(
        uint8_t reg,
        uint8_t value)
{

    uint8_t tx[2];

    tx[0] = reg;
    tx[1] = value;


    return HAL_SPI_Transmit(
        hspi_,
        tx,
        2,
        100
    ) == HAL_OK;

}



bool KX126::readRegister(
        uint8_t reg,
        uint8_t* data,
        uint16_t length)
{
    uint8_t tx = reg | 0x80;


    if(HAL_SPI_Transmit(
            hspi_,
            &tx,
            1,
            100) != HAL_OK)
    {
        return false;
    }


    return HAL_SPI_Receive(
        hspi_,
        data,
        length,
        100
    ) == HAL_OK;
}
