#ifndef MAX30102_H
#define MAX30102_H


#include "IPPGSensor.h"
#include "stm32wbxx_hal.h"

#include "FreeRTOS.h"
#include "semphr.h"


#define MAX30102_ADDR       (0x57 << 1)

#define REG_INTR_STATUS_1   0x00
#define REG_FIFO_WR_PTR     0x04
#define REG_FIFO_OVF_CTR    0x05
#define REG_FIFO_RD_PTR     0x06
#define REG_FIFO_DATA       0x07

#define REG_FIFO_CONFIG     0x08
#define REG_MODE_CONFIG     0x09
#define REG_SPO2_CONFIG     0x0A

#define REG_LED1_PA         0x0C
#define REG_LED2_PA         0x0D



class MAX30102 : public IPPGSensor
{

public:


    explicit MAX30102(
        I2C_HandleTypeDef* hi2c
    );


    bool init() override;


    bool startReadDMA() override;


    bool waitForData(
        uint32_t timeout
    ) override;


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


    I2C_HandleTypeDef* hi2c_;


    volatile bool dmaComplete_;


    uint8_t dmaBuffer_[6];


    SemaphoreHandle_t dmaSemaphore_;



};


#endif
