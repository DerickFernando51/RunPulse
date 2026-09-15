#ifndef IBATTERYSENSOR_H
#define IBATTERYSENSOR_H

#include "ISensor.h"


class IBatterySensor : public ISensor
{
public:

    virtual ~IBatterySensor() = default;


    virtual float getVoltage() = 0;


    virtual float getStateOfCharge() = 0;
};


#endif
