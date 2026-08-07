#include "MAX17048.h"

#define MAX17048_ADDR    (0x36 << 1)

#define REG_VCELL        0x02
#define REG_SOC          0x04

MAX17048::MAX17048(
    I2C_HandleTypeDef* hi2c)
:
hi2c_(hi2c)
{

}

bool MAX17048::init()
{
    return HAL_I2C_IsDeviceReady(
        hi2c_,
        MAX17048_ADDR,
        3,
        100
    ) == HAL_OK;
}

float MAX17048::getVoltage()
{
    uint16_t raw =
        readRegister(REG_VCELL);

    // Upper 12 bits containing the voltage
    raw >>= 4;

    // 1 LSB = 1.25 mV
    return raw * 0.00125f;
}

float MAX17048::getStateOfCharge()
{
    uint16_t raw =
        readRegister(REG_SOC);

    // Integer percentage
    return raw >> 8;


}

uint16_t MAX17048::readRegister(
    uint8_t reg)
{
    uint8_t data[2];

    if(HAL_I2C_Mem_Read(
            hi2c_,
            MAX17048_ADDR,
            reg,
            I2C_MEMADD_SIZE_8BIT,
            data,
            2,
            100) != HAL_OK)
    {
        return 0;
    }

    return
        ((uint16_t)data[0] << 8)
        |
        data[1];
}
