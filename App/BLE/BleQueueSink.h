#pragma once

#include <IBleSink.h>
#include "cmsis_os.h"

class BleQueueSink : public IBleSink
{

public:

    void publish(const BLE_Data_t& data) override
    {

        if (bleQueue != NULL)
        {
            osMessageQueuePut(bleQueue, &data, 0, 0);
        }

    }
};
