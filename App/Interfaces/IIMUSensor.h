#ifndef IIMUSENSOR_H
#define IIMUSENSOR_H

#include "ISensor.h"


struct AccelData
{
    float x;
    float y;
    float z;
};


class IIMUSensor : public ISensor
{
public:

    virtual ~IIMUSensor() = default;


    virtual bool readAcceleration(
        AccelData& data
    ) = 0;
};


#endif
