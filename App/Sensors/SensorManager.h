#include <IBleSink.h>
#include "IPPGSensor.h"
#include "IIMUSensor.h"
#include "IBatterySensor.h"
#include "SensorFrame.h"


class SensorManager
{
public:
    SensorManager(
        IPPGSensor& ppg,
        IIMUSensor& imu,
        IBatterySensor& battery,
        IBleSink& bleSink);

    bool init();
    bool sampleFast(SensorFrame& frame);

private:
    IPPGSensor&     ppg_;
    IIMUSensor&     imu_;
    IBatterySensor& battery_;
    IBleSink&       bleSink_;

    uint8_t batteryCounter_ = 0;
    uint8_t batterySOC_     = 0;
    uint8_t printCounter_   = 0;
};
