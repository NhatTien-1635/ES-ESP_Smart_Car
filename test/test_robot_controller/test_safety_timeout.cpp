#include <cstdint>

#include <unity.h>

#include "../support/fake_motor_driver.h"
#include "../support/test_clock.h"
#include "robot_types.h"

namespace
{

using robot::MotionDirection;
using robot::RobotMode;
using robot::RobotState;
using robot::SafetyReason;
using robot::Timestamp;
using robot::test::FakeMotorDriver;
using robot::test::TestClock;

constexpr Timestamp kCommandTimeout = 100;

struct SafetyFixture
{
    RobotState state;
    FakeMotorDriver motor;
    TestClock clock{1000};

    void startMotion(MotionDirection direction, std::uint8_t speed)
    {
        const auto result = motor.setMotion(direction, speed);
        TEST_ASSERT_EQUAL(robot::MotorResult::APPLIED, result);

        state.mode = RobotMode::MANUAL;
        state.motion = direction;
        state.speed = speed;
        state.lastCommandAt = clock.now();
        state.safetyReason = SafetyReason::NONE;
    }

    void tickTimeout()
    {
        if (state.motion == MotionDirection::STOP
            || clock.now() < state.lastCommandAt
            || clock.now() - state.lastCommandAt < kCommandTimeout)
        {
            return;
        }

        stopFor(SafetyReason::TIMEOUT);
    }

    void disconnect()
    {
        if (state.motion != MotionDirection::STOP)
        {
            stopFor(SafetyReason::DISCONNECTED);
        }
    }

private:
    void stopFor(SafetyReason reason)
    {
        motor.stopAll();
        state.mode = RobotMode::IDLE;
        state.motion = MotionDirection::STOP;
        state.speed = 0;
        state.safetyReason = reason;
    }
};

void test_timeout_stops_motion_and_reports_reason()
{
    SafetyFixture fixture;
    fixture.startMotion(MotionDirection::FORWARD_LEFT, 75);

    fixture.clock.advance(kCommandTimeout - 1);
    fixture.tickTimeout();
    TEST_ASSERT_EQUAL(MotionDirection::FORWARD_LEFT, fixture.state.motion);
    TEST_ASSERT_EQUAL_UINT8(75, fixture.state.speed);
    TEST_ASSERT_EQUAL_UINT(0, fixture.motor.stopCount());

    fixture.clock.advance(1);
    fixture.tickTimeout();

    TEST_ASSERT_EQUAL(RobotMode::IDLE, fixture.state.mode);
    TEST_ASSERT_EQUAL(MotionDirection::STOP, fixture.state.motion);
    TEST_ASSERT_EQUAL_UINT8(0, fixture.state.speed);
    TEST_ASSERT_EQUAL(SafetyReason::TIMEOUT, fixture.state.safetyReason);
    TEST_ASSERT_EQUAL_UINT(1, fixture.motor.stopCount());
}

void test_disconnect_stops_motion_and_reports_reason()
{
    SafetyFixture fixture;
    fixture.startMotion(MotionDirection::BACKWARD_RIGHT, 65);

    fixture.disconnect();

    TEST_ASSERT_EQUAL(RobotMode::IDLE, fixture.state.mode);
    TEST_ASSERT_EQUAL(MotionDirection::STOP, fixture.state.motion);
    TEST_ASSERT_EQUAL_UINT8(0, fixture.state.speed);
    TEST_ASSERT_EQUAL(SafetyReason::DISCONNECTED,
                      fixture.state.safetyReason);
    TEST_ASSERT_EQUAL_UINT(1, fixture.motor.stopCount());
}

void test_repeated_safety_events_are_idempotent()
{
    SafetyFixture fixture;
    fixture.startMotion(MotionDirection::RIGHT, 40);

    fixture.clock.advance(kCommandTimeout);
    fixture.tickTimeout();
    fixture.tickTimeout();
    fixture.disconnect();

    TEST_ASSERT_EQUAL(RobotMode::IDLE, fixture.state.mode);
    TEST_ASSERT_EQUAL(MotionDirection::STOP, fixture.state.motion);
    TEST_ASSERT_EQUAL_UINT8(0, fixture.state.speed);
    TEST_ASSERT_EQUAL(SafetyReason::TIMEOUT, fixture.state.safetyReason);
    TEST_ASSERT_EQUAL_UINT(1, fixture.motor.stopCount());
}

} // namespace

void run_safety_timeout_tests()
{
    RUN_TEST(test_timeout_stops_motion_and_reports_reason);
    RUN_TEST(test_disconnect_stops_motion_and_reports_reason);
    RUN_TEST(test_repeated_safety_events_are_idempotent);
}
