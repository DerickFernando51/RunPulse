#include "MAX30102_Callback.h"
#include "MAX30102.h"


static MAX30102* instance = nullptr;


void MAX30102_RegisterInstance(void* sensor)
{
    instance = (MAX30102*)sensor;
}



void MAX30102_I2C_Callback(void* hi2c)
{
    if(instance)
    {
        instance->rxCompleteCallback(
            (I2C_HandleTypeDef*)hi2c
        );
    }
}
