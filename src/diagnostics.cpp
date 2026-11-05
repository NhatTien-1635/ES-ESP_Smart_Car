#include "diagnostics.h"

namespace robot
{

const char* diagnosticCodeName(DiagnosticCode code)
{
    switch (code)
    {
    case DiagnosticCode::NONE:
        return "NONE";
    case DiagnosticCode::INVALID_COMMAND:
        return "INVALID_COMMAND";
    case DiagnosticCode::UNSUPPORTED_DIRECTION:
        return "UNSUPPORTED_DIRECTION";
    case DiagnosticCode::MOTOR_HARDWARE_ERROR:
        return "MOTOR_HARDWARE_ERROR";
    case DiagnosticCode::FILESYSTEM_ERROR:
        return "FILESYSTEM_ERROR";
    case DiagnosticCode::WIFI_ERROR:
        return "WIFI_ERROR";
    case DiagnosticCode::TASK_ERROR:
        return "TASK_ERROR";
    case DiagnosticCode::SENSOR_ERROR:
        return "SENSOR_ERROR";
    }

    return "UNKNOWN_DIAGNOSTIC";
}

const char* diagnosticMessage(DiagnosticCode code)
{
    switch (code)
    {
    case DiagnosticCode::NONE:
        return "No diagnostic error.";
    case DiagnosticCode::INVALID_COMMAND:
        return "The command is invalid and was rejected.";
    case DiagnosticCode::UNSUPPORTED_DIRECTION:
        return "The requested direction is not supported by the motor adapter.";
    case DiagnosticCode::MOTOR_HARDWARE_ERROR:
        return "The motor driver reported a hardware error.";
    case DiagnosticCode::FILESYSTEM_ERROR:
        return "The filesystem could not be initialized or accessed.";
    case DiagnosticCode::WIFI_ERROR:
        return "The Wi-Fi access point could not be initialized.";
    case DiagnosticCode::TASK_ERROR:
        return "A required firmware task could not be created or started.";
    case DiagnosticCode::SENSOR_ERROR:
        return "One or more sensor readings are unavailable or invalid.";
    }

    return "An unknown diagnostic error occurred.";
}

DiagnosticEvent diagnosticForCommand(CommandStatus status)
{
    switch (status)
    {
    case CommandStatus::ACCEPTED:
        return {DiagnosticCode::NONE, false};
    case CommandStatus::INVALID:
        return {DiagnosticCode::INVALID_COMMAND, true};
    case CommandStatus::UNSUPPORTED:
        return {DiagnosticCode::UNSUPPORTED_DIRECTION, true};
    }

    return {DiagnosticCode::INVALID_COMMAND, true};
}

DiagnosticEvent diagnosticForMotor(MotorResult result)
{
    switch (result)
    {
    case MotorResult::APPLIED:
    case MotorResult::STOPPED:
        return {DiagnosticCode::NONE, false};
    case MotorResult::UNSUPPORTED_DIRECTION:
        return {DiagnosticCode::UNSUPPORTED_DIRECTION, true};
    case MotorResult::HARDWARE_ERROR:
        return {DiagnosticCode::MOTOR_HARDWARE_ERROR, true};
    case MotorResult::INVALID_SPEED:
        return {DiagnosticCode::INVALID_COMMAND, true};
    }

    return {DiagnosticCode::MOTOR_HARDWARE_ERROR, true};
}

DiagnosticEvent diagnosticForSensor(SensorUpdateResult result)
{
    switch (result)
    {
    case SensorUpdateResult::UPDATED:
        return {DiagnosticCode::NONE, false};
    case SensorUpdateResult::PARTIAL:
    case SensorUpdateResult::FAILED:
        return {DiagnosticCode::SENSOR_ERROR, true};
    }

    return {DiagnosticCode::SENSOR_ERROR, true};
}

} // namespace robot
