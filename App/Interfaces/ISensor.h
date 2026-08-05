#ifndef ISENSOR_H
#define ISENSOR_H

class ISensor
{
public:
    virtual ~ISensor() = default;

    virtual bool init() = 0;
};

#endif
