#ifndef IPPGSENSOR_H
#define IPPGSENSOR_H

#include <stdint.h>


struct PPGData
{
    uint32_t red;
    uint32_t ir;
};


class IPPGSensor
{
public:

    virtual bool init() = 0;

    // Start DMA read
    virtual bool startReadDMA() = 0;

    // Check if DMA is finished
    virtual bool dataReady() const = 0;

    // Get number of samples available
    virtual uint8_t availableSamples() const = 0;

    // Get next sample
    virtual bool getSample(
        PPGData& data
    ) = 0;

    // Check if finger is present
    virtual bool fingerPresent(
        const PPGData& data
    ) const = 0;

    virtual ~IPPGSensor() {}
};


#endif
