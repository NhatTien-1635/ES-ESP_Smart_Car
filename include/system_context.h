#ifndef SYSTEM_CONTEXT_H
#define SYSTEM_CONTEXT_H

#include "motor_driver.h"
#include "sensors.h"
#include "telemetry.h"

namespace robot
{

struct SystemContext
{
    // These are non-owning dependencies. Hardware lifetime is managed by setup code.
    MotorDriver* motorDriver = nullptr;
    SensorAdapter* sensorAdapter = nullptr;
    TelemetryProvider* telemetryProvider = nullptr;
};

} // namespace robot

#endif // SYSTEM_CONTEXT_H
