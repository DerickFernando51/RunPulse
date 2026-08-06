#include "MAX30102.h"

#include "usbd_cdc_if.h"


#define MAX30102_ADDR      (0x57 << 1)

#define REG_FIFO_DATA      0x07
#define REG_MODE_CONFIG    0x09
#define REG_FIFO_CONFIG    0x08
#define REG_SPO2_CONFIG    0x0A
#define REG_LED1_PA        0x0C
#define REG_LED2_PA        0x0D


MAX30102::MAX30102(
        I2C_HandleTypeDef* hi2c)
:
hi2c_(hi2c),
dmaComplete_(false),
dmaSemaphore_(nullptr)
{

}

bool MAX30102::init()
{

    // Create semaphore first
    dmaSemaphore_ = xSemaphoreCreateBinary();

    if(dmaSemaphore_ == nullptr)
    {
        return false;
    }


    // Check I2C device
    if(HAL_I2C_IsDeviceReady(
            hi2c_,
            MAX30102_ADDR,
            3,
            100) != HAL_OK)
    {
        return false;
    }



    // Reset
    if(!writeRegister(
            REG_MODE_CONFIG,
            0x40))
    {
        return false;
    }


    HAL_Delay(100);



    // FIFO configuration
    if(!writeRegister(
            REG_FIFO_CONFIG,
            0x4F))
    {
        return false;
    }



    // SpO2 mode
    if(!writeRegister(
            REG_MODE_CONFIG,
            0x03))
    {
        return false;
    }



    // ADC configuration
    if(!writeRegister(
            REG_SPO2_CONFIG,
            0x27))
    {
        return false;
    }



    // LED currents

    if(!writeRegister(REG_LED1_PA,0x24))
        return false;


    if(!writeRegister(REG_LED2_PA,0x24))
        return false;


    return true;
}


bool MAX30102::startReadDMA()
{

    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read_DMA(
        hi2c_,
        MAX30102_ADDR,
        REG_FIFO_DATA,
        I2C_MEMADD_SIZE_8BIT,
        dmaBuffer_,
        6
    );


    if(status != HAL_OK)
    {
        CDC_Transmit_FS(
            (uint8_t*)"DMA START FAIL\r\n",
            16
        );

        return false;
    }


//    CDC_Transmit_FS(
//        (uint8_t*)"DMA START OK\r\n",
//        15
//    );
//

    return true;
}




bool MAX30102::getSample(PPGData& data)
{

    data.red =
        ((uint32_t)(dmaBuffer_[0] & 0x03) << 16)
        |
        ((uint32_t)dmaBuffer_[1] << 8)
        |
        dmaBuffer_[2];


    data.ir =
        ((uint32_t)(dmaBuffer_[3] & 0x03) << 16)
        |
        ((uint32_t)dmaBuffer_[4] << 8)
        |
        dmaBuffer_[5];


    return true;
}





bool MAX30102::fingerPresent(
        const PPGData& data) const
{

    return data.ir > 50000;

}


void MAX30102::rxCompleteCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c != hi2c_)
        return;


//    CDC_Transmit_FS(
//        (uint8_t*)"MAX DMA CALLBACK\r\n",
//        18
//    );


    xSemaphoreGiveFromISR(
        dmaSemaphore_,
        NULL
    );
}


bool MAX30102::waitForData(uint32_t timeout)
{
    return xSemaphoreTake(
        dmaSemaphore_,
        pdMS_TO_TICKS(timeout)
    ) == pdTRUE;
}


bool MAX30102::writeRegister(
        uint8_t reg,
        uint8_t value)
{

    uint8_t data[2];

    data[0] = reg;
    data[1] = value;



    return
    HAL_I2C_Master_Transmit(
            hi2c_,
            MAX30102_ADDR,
            data,
            2,
            100
    )
    == HAL_OK;

}
