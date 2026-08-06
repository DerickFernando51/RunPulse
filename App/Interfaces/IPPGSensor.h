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


    virtual bool startReadDMA() = 0;


    virtual bool waitForData(
        uint32_t timeout
    ) = 0;


    virtual bool getSample(
        PPGData& data
    ) = 0;


    virtual bool fingerPresent(
        const PPGData& data
    ) const = 0;


    virtual ~IPPGSensor(){}

};


#endif
