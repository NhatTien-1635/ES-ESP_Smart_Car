#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <cstdint>

#include "motor_driver.h"
#include "robot_types.h"
#include "sensors.h"

namespace robot
{

enum class DiagnosticCode : std::uint8_t
{
    NONE,
    INVALID_COMMAND,
    UNSUPPORTED_DIRECTION,
    MOTOR_HARDWARE_ERROR,
    FILESYSTEM_ERROR,
    WIFI_ERROR,
    TASK_ERROR,
    SENSOR_ERROR
};

struct DiagnosticEvent
{
    DiagnosticCode code = DiagnosticCode::NONE;
    bool failure = false;
};

// Convert a diagnostic code to a stable identifier for logs and API responses.
const char* diagnosticCodeName(DiagnosticCode code);

// Return a human-readable English message without hiding the failure.
const char* diagnosticMessage(DiagnosticCode code);

// Map command validation outcomes to diagnostic events.
DiagnosticEvent diagnosticForCommand(CommandStatus status);

// Map motor adapter outcomes to diagnostic events.
DiagnosticEvent diagnosticForMotor(MotorResult result);

// Map sensor adapter outcomes to diagnostic events.
DiagnosticEvent diagnosticForSensor(SensorUpdateResult result);

} // namespace robot

#endif // DIAGNOSTICS_H
