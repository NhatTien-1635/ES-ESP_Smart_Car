#include "motor_driver.h"

#include <Arduino.h>

#include <algorithm>

namespace robot
{

namespace
{

constexpr std::int16_t kPwmMax = 255;

std::int16_t signedSpeed(std::uint8_t speed, bool forward)
{
    const std::int16_t pwm =
        static_cast<std::int16_t>((speed * kPwmMax) / kMaxSpeedPercent);
    return forward ? pwm : static_cast<std::int16_t>(-pwm);
}

void configureMotorPins(std::uint8_t in1,
                        std::uint8_t in2,
                        std::uint8_t enable)
{
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(enable, OUTPUT);
}

} // namespace

L298NMotorDriver::L298NMotorDriver(const MotorPins& pins)
    : pins_(pins)
{
}

void L298NMotorDriver::begin()
{
    configureMotorPins(pins_.frontLeftIn1,
                        pins_.frontLeftIn2,
                        pins_.frontLeftEnable);
    configureMotorPins(pins_.frontRightIn1,
                        pins_.frontRightIn2,
                        pins_.frontRightEnable);
    configureMotorPins(pins_.rearLeftIn1,
                        pins_.rearLeftIn2,
                        pins_.rearLeftEnable);
    configureMotorPins(pins_.rearRightIn1,
                        pins_.rearRightIn2,
                        pins_.rearRightEnable);
    stopAll();
}

MotorResult L298NMotorDriver::setMotion(MotionDirection direction,
                                        std::uint8_t speed)
{
    if (speed > kMaxSpeedPercent)
    {
        stopAll();
        return MotorResult::INVALID_SPEED;
    }

    if (direction == MotionDirection::STOP)
    {
        stopAll();
        return MotorResult::STOPPED;
    }

    const WheelSpeeds wheels = mixMotion(direction, speed);
    if (wheels.frontLeft == 0 && wheels.frontRight == 0
        && wheels.rearLeft == 0 && wheels.rearRight == 0)
    {
        stopAll();
        return MotorResult::UNSUPPORTED_DIRECTION;
    }

    applyWheel(pins_.frontLeftIn1,
               pins_.frontLeftIn2,
               pins_.frontLeftEnable,
               wheels.frontLeft);
    applyWheel(pins_.frontRightIn1,
               pins_.frontRightIn2,
               pins_.frontRightEnable,
               wheels.frontRight);
    applyWheel(pins_.rearLeftIn1,
               pins_.rearLeftIn2,
               pins_.rearLeftEnable,
               wheels.rearLeft);
    applyWheel(pins_.rearRightIn1,
               pins_.rearRightIn2,
               pins_.rearRightEnable,
               wheels.rearRight);
    return MotorResult::APPLIED;
}

void L298NMotorDriver::stopAll()
{
    applyWheel(pins_.frontLeftIn1,
               pins_.frontLeftIn2,
               pins_.frontLeftEnable,
               0);
    applyWheel(pins_.frontRightIn1,
               pins_.frontRightIn2,
               pins_.frontRightEnable,
               0);
    applyWheel(pins_.rearLeftIn1,
               pins_.rearLeftIn2,
               pins_.rearLeftEnable,
               0);
    applyWheel(pins_.rearRightIn1,
               pins_.rearRightIn2,
               pins_.rearRightEnable,
               0);
}

WheelSpeeds L298NMotorDriver::mixMotion(MotionDirection direction,
                                        std::uint8_t speed)
{
    switch (direction)
    {
    case MotionDirection::FORWARD:
        return {signedSpeed(speed, true),
                signedSpeed(speed, true),
                signedSpeed(speed, true),
                signedSpeed(speed, true)};
    case MotionDirection::BACKWARD:
        return {signedSpeed(speed, false),
                signedSpeed(speed, false),
                signedSpeed(speed, false),
                signedSpeed(speed, false)};
    case MotionDirection::LEFT:
        return {signedSpeed(speed, false),
                signedSpeed(speed, true),
                signedSpeed(speed, true),
                signedSpeed(speed, false)};
    case MotionDirection::RIGHT:
        return {signedSpeed(speed, true),
                signedSpeed(speed, false),
                signedSpeed(speed, false),
                signedSpeed(speed, true)};
    case MotionDirection::FORWARD_LEFT:
        return {0,
                signedSpeed(speed, true),
                signedSpeed(speed, true),
                0};
    case MotionDirection::FORWARD_RIGHT:
        return {signedSpeed(speed, true),
                0,
                0,
                signedSpeed(speed, true)};
    case MotionDirection::BACKWARD_LEFT:
        return {0,
                signedSpeed(speed, false),
                signedSpeed(speed, false),
                0};
    case MotionDirection::BACKWARD_RIGHT:
        return {signedSpeed(speed, false),
                0,
                0,
                signedSpeed(speed, false)};
    case MotionDirection::STOP:
        return {};
    }

    return {};
}

void L298NMotorDriver::applyWheel(std::uint8_t in1,
                                  std::uint8_t in2,
                                  std::uint8_t enable,
                                  std::int16_t speed)
{
    const std::int16_t magnitude =
        std::min<std::int16_t>(std::abs(speed), kPwmMax);
    if (speed > 0)
    {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
    }
    else if (speed < 0)
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
    }
    else
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
    }
    analogWrite(enable, magnitude);
}

} // namespace robot
