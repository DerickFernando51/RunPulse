#ifndef ISENSOR_H
#define ISENSOR_H

#include <stdint.h>


class ISensor
{

public:

    virtual bool init() = 0;


    virtual ~ISensor(){}

};


#endif
