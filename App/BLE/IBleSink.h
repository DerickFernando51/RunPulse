#pragma once

#include "tasks.h"

class IBleSink
{

public:

    virtual ~IBleSink() = default;

    virtual void publish(const BLE_Data_t& data) = 0;

};
