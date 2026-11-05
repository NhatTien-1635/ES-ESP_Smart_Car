#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <cstdint>

#include "robot_types.h"

namespace robot
{

enum class MotorResult : std::uint8_t
{
    APPLIED,
    STOPPED,
    INVALID_SPEED,
    UNSUPPORTED_DIRECTION,
    HARDWARE_ERROR
};

class MotorDriver
{
public:
    virtual ~MotorDriver() = default;

    // Apply a motion request without exposing chassis or GPIO details.
    virtual MotorResult setMotion(MotionDirection direction,
                                  std::uint8_t speed) = 0;

    // Stop every motor channel and leave the driver in a safe state.
    virtual void stopAll() = 0;
};

} // namespace robot

#endif // MOTOR_DRIVER_H
