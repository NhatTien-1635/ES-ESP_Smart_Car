#ifndef FAKE_MOTOR_DRIVER_H
#define FAKE_MOTOR_DRIVER_H

#include <array>
#include <cstddef>
#include <cstdint>

#include "motor_driver.h"

namespace robot::test
{

struct MotionRequest
{
    MotionDirection direction = MotionDirection::STOP;
    std::uint8_t speed = 0;
};

class FakeMotorDriver final : public MotorDriver
{
public:
    FakeMotorDriver()
    {
        supported_.fill(true);
    }

    MotorResult setMotion(MotionDirection direction,
                          std::uint8_t speed) override
    {
        lastRequest_ = {direction, speed};

        if (speed > kMaxSpeedPercent)
        {
            lastResult_ = MotorResult::INVALID_SPEED;
            return lastResult_;
        }

        if (!supported_[toIndex(direction)])
        {
            lastResult_ = MotorResult::UNSUPPORTED_DIRECTION;
            return lastResult_;
        }

        if (direction == MotionDirection::STOP)
        {
            stopAll();
            lastResult_ = MotorResult::STOPPED;
            return lastResult_;
        }

        lastResult_ = MotorResult::APPLIED;
        return lastResult_;
    }

    void stopAll() override
    {
        lastRequest_ = {MotionDirection::STOP, 0};
        stopCount_++;
    }

    void setSupported(MotionDirection direction, bool supported)
    {
        supported_[toIndex(direction)] = supported;
    }

    const MotionRequest& lastRequest() const
    {
        return lastRequest_;
    }

    MotorResult lastResult() const
    {
        return lastResult_;
    }

    std::size_t stopCount() const
    {
        return stopCount_;
    }

private:
    static std::size_t toIndex(MotionDirection direction)
    {
        return static_cast<std::size_t>(direction);
    }

    std::array<bool, 9> supported_{};
    MotionRequest lastRequest_{};
    MotorResult lastResult_ = MotorResult::STOPPED;
    std::size_t stopCount_ = 0;
};

} // namespace robot::test

#endif // FAKE_MOTOR_DRIVER_H
