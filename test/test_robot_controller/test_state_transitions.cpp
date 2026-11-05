#include <cstdint>

#include <unity.h>

#include "../support/fake_motor_driver.h"
#include "robot_types.h"

namespace
{

using robot::CommandKind;
using robot::ControlCommand;
using robot::MotionDirection;
using robot::RobotMode;
using robot::RobotState;
using robot::test::FakeMotorDriver;

struct TransitionFixture
{
    RobotState state;
    FakeMotorDriver motor;

    void move(MotionDirection direction, std::uint8_t speed)
    {
        const bool replacingMotion = state.motion != MotionDirection::STOP;
        if (replacingMotion)
        {
            motor.stopAll();
        }

        const auto result = motor.setMotion(direction, speed);
        if (result != robot::MotorResult::APPLIED)
        {
            motor.stopAll();
            state.mode = RobotMode::IDLE;
            state.motion = MotionDirection::STOP;
            state.speed = 0;
            return;
        }

        state.mode = RobotMode::MANUAL;
        state.motion = direction;
        state.speed = speed;
        state.safetyReason = robot::SafetyReason::NONE;
    }

    void setMode(RobotMode mode)
    {
        motor.stopAll();
        state.mode = mode;
        state.motion = MotionDirection::STOP;
        state.speed = 0;
    }

    void stop()
    {
        motor.stopAll();
        state.mode = RobotMode::IDLE;
        state.motion = MotionDirection::STOP;
        state.speed = 0;
    }
};

ControlCommand makeCommand(CommandKind kind,
                           MotionDirection direction,
                           std::uint8_t speed)
{
    ControlCommand command;
    command.kind = kind;
    command.direction = direction;
    command.speed = speed;
    command.hasDirection = kind == CommandKind::MOVE;
    command.hasSpeed = kind == CommandKind::MOVE;
    return command;
}

void test_idle_move_enters_manual_mode()
{
    TransitionFixture fixture;
    const ControlCommand command =
        makeCommand(CommandKind::MOVE, MotionDirection::FORWARD, 60);

    fixture.move(command.direction, command.speed);

    TEST_ASSERT_EQUAL(RobotMode::MANUAL, fixture.state.mode);
    TEST_ASSERT_EQUAL(MotionDirection::FORWARD, fixture.state.motion);
    TEST_ASSERT_EQUAL_UINT8(60, fixture.state.speed);
    TEST_ASSERT_EQUAL_UINT(0, fixture.motor.stopCount());
}

void test_cardinal_to_diagonal_replaces_old_motion()
{
    TransitionFixture fixture;
    fixture.move(MotionDirection::FORWARD, 70);

    fixture.move(MotionDirection::FORWARD_LEFT, 80);

    TEST_ASSERT_EQUAL(RobotMode::MANUAL, fixture.state.mode);
    TEST_ASSERT_EQUAL(MotionDirection::FORWARD_LEFT, fixture.state.motion);
    TEST_ASSERT_EQUAL_UINT8(80, fixture.state.speed);
    TEST_ASSERT_EQUAL_UINT(1, fixture.motor.stopCount());
    TEST_ASSERT_EQUAL(MotionDirection::FORWARD_LEFT,
                      fixture.motor.lastRequest().direction);
}

void test_diagonal_to_diagonal_replaces_old_motion()
{
    TransitionFixture fixture;
    fixture.move(MotionDirection::FORWARD_LEFT, 80);

    fixture.move(MotionDirection::BACKWARD_RIGHT, 50);

    TEST_ASSERT_EQUAL(MotionDirection::BACKWARD_RIGHT, fixture.state.motion);
    TEST_ASSERT_EQUAL_UINT8(50, fixture.state.speed);
    TEST_ASSERT_EQUAL_UINT(1, fixture.motor.stopCount());
    TEST_ASSERT_EQUAL(MotionDirection::BACKWARD_RIGHT,
                      fixture.motor.lastRequest().direction);
}

void test_manual_to_line_tracking_stops_manual_motion()
{
    TransitionFixture fixture;
    fixture.move(MotionDirection::RIGHT, 55);

    fixture.setMode(RobotMode::LINE_TRACKING);

    TEST_ASSERT_EQUAL(RobotMode::LINE_TRACKING, fixture.state.mode);
    TEST_ASSERT_EQUAL(MotionDirection::STOP, fixture.state.motion);
    TEST_ASSERT_EQUAL_UINT8(0, fixture.state.speed);
    TEST_ASSERT_EQUAL_UINT(1, fixture.motor.stopCount());
    TEST_ASSERT_EQUAL(MotionDirection::STOP,
                      fixture.motor.lastRequest().direction);
}

void test_stop_is_idempotent_and_clears_previous_motion()
{
    TransitionFixture fixture;
    fixture.move(MotionDirection::BACKWARD_LEFT, 65);

    fixture.stop();
    fixture.stop();

    TEST_ASSERT_EQUAL(RobotMode::IDLE, fixture.state.mode);
    TEST_ASSERT_EQUAL(MotionDirection::STOP, fixture.state.motion);
    TEST_ASSERT_EQUAL_UINT8(0, fixture.state.speed);
    TEST_ASSERT_EQUAL_UINT(2, fixture.motor.stopCount());
    TEST_ASSERT_EQUAL(MotionDirection::STOP,
                      fixture.motor.lastRequest().direction);
}

} // namespace

void run_state_transition_tests()
{
    RUN_TEST(test_idle_move_enters_manual_mode);
    RUN_TEST(test_cardinal_to_diagonal_replaces_old_motion);
    RUN_TEST(test_diagonal_to_diagonal_replaces_old_motion);
    RUN_TEST(test_manual_to_line_tracking_stops_manual_motion);
    RUN_TEST(test_stop_is_idempotent_and_clears_previous_motion);
}
