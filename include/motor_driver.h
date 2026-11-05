#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <array>
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

struct MotorPins
{
    std::uint8_t frontLeftIn1 = 11;
    std::uint8_t frontLeftIn2 = 12;
    std::uint8_t frontLeftEnable = 13;
    std::uint8_t frontRightIn1 = 14;
    std::uint8_t frontRightIn2 = 15;
    std::uint8_t frontRightEnable = 16;
    std::uint8_t rearLeftIn1 = 17;
    std::uint8_t rearLeftIn2 = 18;
    std::uint8_t rearLeftEnable = 21;
    std::uint8_t rearRightIn1 = 38;
    std::uint8_t rearRightIn2 = 39;
    std::uint8_t rearRightEnable = 40;
};

struct WheelSpeeds
{
    std::int16_t frontLeft = 0;
    std::int16_t frontRight = 0;
    std::int16_t rearLeft = 0;
    std::int16_t rearRight = 0;
};

class L298NMotorDriver final : public MotorDriver
{
public:
    explicit L298NMotorDriver(const MotorPins& pins = {});

    void begin();

    MotorResult setMotion(MotionDirection direction,
                          std::uint8_t speed) override;
    void stopAll() override;

    static WheelSpeeds mixMotion(MotionDirection direction,
                                 std::uint8_t speed);

private:
    void applyWheel(std::uint8_t in1,
                    std::uint8_t in2,
                    std::uint8_t enable,
                    std::int16_t speed);

    MotorPins pins_;
};

} // namespace robot

#endif // MOTOR_DRIVER_H
