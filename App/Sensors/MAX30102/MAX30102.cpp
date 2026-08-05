#include "MAX30102.h"


#define MAX30102_ADDR (0x57 << 1)

#define REG_MODE_CONFIG 0x09
#define REG_FIFO_DATA   0x07



MAX30102::MAX30102(
        I2C_HandleTypeDef* hi2c)
        :
        hi2c_(hi2c),
        dmaComplete_(false)
{

}



bool MAX30102::init()
{

    writeRegister(
        REG_MODE_CONFIG,
        0x03
    );


    return true;
}



bool MAX30102::startReadDMA()
{

    dmaComplete_ = false;


    return HAL_I2C_Mem_Read_DMA(
        hi2c_,
        MAX30102_ADDR,
        REG_FIFO_DATA,
        I2C_MEMADD_SIZE_8BIT,
        dmaBuffer_,
        6
    ) == HAL_OK;

}



bool MAX30102::dataReady() const
{
    return dmaComplete_;
}



bool MAX30102::getSample(
        PPGData& data)
{

    if(!dmaComplete_)
        return false;


    data.red =
        ((dmaBuffer_[0] & 0x03) << 16)
        |
        (dmaBuffer_[1] << 8)
        |
        dmaBuffer_[2];


    data.ir =
        ((dmaBuffer_[3] & 0x03) << 16)
        |
        (dmaBuffer_[4] << 8)
        |
        dmaBuffer_[5];


    dmaComplete_ = false;


    return true;
}



bool MAX30102::fingerPresent(
        const PPGData& data) const
{
    return data.ir > 50000;
}



void MAX30102::rxCompleteCallback(
        I2C_HandleTypeDef* hi2c)
{

    if(hi2c == hi2c_)
    {
        dmaComplete_ = true;
    }

}



bool MAX30102::writeRegister(
        uint8_t reg,
        uint8_t value)
{

    uint8_t data[2] =
    {
        reg,
        value
    };


    return HAL_I2C_Master_Transmit(
        hi2c_,
        MAX30102_ADDR,
        data,
        2,
        100
    ) == HAL_OK;

}
