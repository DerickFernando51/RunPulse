#ifndef IPPGSENSOR_H
#define IPPGSENSOR_H

#include "ISensor.h"


struct PPGData
{
    uint32_t red;
    uint32_t ir;
};


class IPPGSensor : public ISensor
{
public:

    virtual ~IPPGSensor() = default;


    // Start non-blocking DMA read
    virtual bool startReadDMA() = 0;


    // Check if DMA finished
    virtual bool dataReady() const = 0;


    // Retrieve processed sample
    virtual bool getSample(
        PPGData& data
    ) = 0;


    // Check finger contact
    virtual bool fingerPresent(
        const PPGData& data
    ) const = 0;
};


#endif
