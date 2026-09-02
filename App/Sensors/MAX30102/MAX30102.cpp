#include "MAX30102.h"


MAX30102::MAX30102(
        I2C_HandleTypeDef* hi2c)
:
hi2c_(hi2c),
dmaComplete_(false),
samplesInBuffer_(0),
sampleIndex_(0)
{
}


bool MAX30102::init()
{
    // Check I2C device
    if(HAL_I2C_IsDeviceReady(
            hi2c_,
            MAX30102_ADDR,
            3,
            100) != HAL_OK)
    {
        return false;
    }


    // Reset MAX30102
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


    // ADC / sample configuration
    if(!writeRegister(
            REG_SPO2_CONFIG,
            0x2B))
    {
        return false;
    }


    // RED LED current
    if(!writeRegister(
            REG_LED1_PA,
            0x24))
    {
        return false;
    }


    // IR LED current
    if(!writeRegister(
            REG_LED2_PA,
            0x24))
    {
        return false;
    }


    dmaComplete_ = false;

    samplesInBuffer_ = 0;

    sampleIndex_ = 0;


    return true;
}


bool MAX30102::startReadDMA()
{
    // Do not start another DMA transfer
    // while the previous one is running
    if(dmaComplete_)
    {
        return false;
    }


    // Check how many samples are in FIFO
    uint8_t fifoSamples = getFifoSamples();


    // Need at least 4 samples
    if(fifoSamples < MAX30102_SAMPLES_PER_BATCH)
    {
        return false;
    }


    // Clear previous batch state
    samplesInBuffer_ = 0;

    sampleIndex_ = 0;

    dmaComplete_ = false;


    // Read 4 samples from FIFO
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read_DMA(
        hi2c_,
        MAX30102_ADDR,
        REG_FIFO_DATA,
        I2C_MEMADD_SIZE_8BIT,
        dmaBuffer_,
        MAX30102_DMA_BUFFER_SIZE
    );


    if(status != HAL_OK)
    {
        return false;
    }


    return true;
}


bool MAX30102::dataReady() const
{
    return dmaComplete_;
}


uint8_t MAX30102::availableSamples() const
{
    if(!dmaComplete_)
    {
        return 0;
    }


    if(sampleIndex_ >= samplesInBuffer_)
    {
        return 0;
    }


    return samplesInBuffer_ - sampleIndex_;
}


bool MAX30102::getSample(
        PPGData& data)
{
    if(!dmaComplete_)
    {
        return false;
    }


    if(sampleIndex_ >= samplesInBuffer_)
    {
        return false;
    }


    uint8_t offset =
        sampleIndex_ * MAX30102_BYTES_PER_SAMPLE;


    // RED
    data.red =
        ((uint32_t)(dmaBuffer_[offset] & 0x03) << 16)
        |
        ((uint32_t)dmaBuffer_[offset + 1] << 8)
        |
        dmaBuffer_[offset + 2];


    // IR
    data.ir =
        ((uint32_t)(dmaBuffer_[offset + 3] & 0x03) << 16)
        |
        ((uint32_t)dmaBuffer_[offset + 4] << 8)
        |
        dmaBuffer_[offset + 5];


    sampleIndex_++;


    // All samples from this batch have been read
    if(sampleIndex_ >= samplesInBuffer_)
    {
        dmaComplete_ = false;

        samplesInBuffer_ = 0;

        sampleIndex_ = 0;
    }


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
    if(hi2c != hi2c_)
    {
        return;
    }


    // DMA transfer is complete
    dmaComplete_ = true;

    samplesInBuffer_ =
        MAX30102_SAMPLES_PER_BATCH;
}


bool MAX30102::writeRegister(
        uint8_t reg,
        uint8_t value)
{
    uint8_t data[2];

    data[0] = reg;
    data[1] = value;


    return HAL_I2C_Master_Transmit(
        hi2c_,
        MAX30102_ADDR,
        data,
        2,
        100
    ) == HAL_OK;
}


bool MAX30102::readRegister(
        uint8_t reg,
        uint8_t& value)
{
    return HAL_I2C_Mem_Read(
        hi2c_,
        MAX30102_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &value,
        1,
        100
    ) == HAL_OK;
}


uint8_t MAX30102::getFifoSamples()
{
    uint8_t writePointer;
    uint8_t readPointer;


    if(!readRegister(
            REG_FIFO_WR_PTR,
            writePointer))
    {
        return 0;
    }


    if(!readRegister(
            REG_FIFO_RD_PTR,
            readPointer))
    {
        return 0;
    }


    writePointer &= 0x1F;

    readPointer &= 0x1F;


    if(writePointer >= readPointer)
    {
        return writePointer - readPointer;
    }


    return (32 - readPointer) + writePointer;
}

