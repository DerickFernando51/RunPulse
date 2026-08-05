#ifndef IBATTERYMONITOR_H
#define IBATTERYMONITOR_H

#include "ISensor.h"


class IBatteryMonitor : public ISensor
{
public:

    virtual ~IBatteryMonitor() = default;


    virtual float getVoltage() = 0;


    virtual float getStateOfCharge() = 0;
};


#endif
