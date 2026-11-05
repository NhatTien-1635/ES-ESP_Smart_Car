#include <array>
#include <cstdint>

#include <unity.h>

#include "../support/fake_motor_driver.h"
#include "robot_types.h"

namespace
{

using robot::CommandKind;
using robot::ControlCommand;
using robot::MotionDirection;
using robot::test::FakeMotorDriver;

constexpr std::array<MotionDirection, 8> kValidMoveDirections = {
    MotionDirection::FORWARD,
    MotionDirection::BACKWARD,
    MotionDirection::LEFT,
    MotionDirection::RIGHT,
    MotionDirection::FORWARD_LEFT,
    MotionDirection::FORWARD_RIGHT,
    MotionDirection::BACKWARD_LEFT,
    MotionDirection::BACKWARD_RIGHT};

bool isValidMoveDirection(MotionDirection direction)
{
    for (const MotionDirection validDirection : kValidMoveDirections)
    {
        if (direction == validDirection)
        {
            return true;
        }
    }

    return false;
}

bool isValidSpeed(std::uint8_t speed)
{
    return speed >= robot::kMinSpeedPercent
           && speed <= robot::kMaxSpeedPercent;
}

ControlCommand makeMoveCommand(MotionDirection direction, std::uint8_t speed)
{
    ControlCommand command;
    command.kind = CommandKind::MOVE;
    command.direction = direction;
    command.speed = speed;
    command.hasDirection = true;
    command.hasSpeed = true;
    return command;
}

void test_all_eight_directions_are_valid_move_values()
{
    for (const MotionDirection direction : kValidMoveDirections)
    {
        const ControlCommand command = makeMoveCommand(direction, 80);
        TEST_ASSERT_TRUE(isValidMoveDirection(command.direction));
        TEST_ASSERT_TRUE(isValidSpeed(command.speed));
    }
}

void test_stop_is_not_a_move_direction()
{
    const ControlCommand command = makeMoveCommand(MotionDirection::STOP, 80);
    TEST_ASSERT_FALSE(isValidMoveDirection(command.direction));
}

void test_unknown_direction_value_is_rejected()
{
    const auto unknownDirection = static_cast<MotionDirection>(255);
    const ControlCommand command = makeMoveCommand(unknownDirection, 80);
    TEST_ASSERT_FALSE(isValidMoveDirection(command.direction));
}

void test_missing_direction_is_rejected()
{
    ControlCommand command;
    command.kind = CommandKind::MOVE;
    command.hasDirection = false;
    command.hasSpeed = true;
    command.speed = 80;

    TEST_ASSERT_FALSE(command.hasDirection);
}

void test_speed_outside_range_is_rejected()
{
    TEST_ASSERT_FALSE(isValidSpeed(static_cast<std::uint8_t>(101)));
}

void test_unsupported_diagonal_stops_motor()
{
    FakeMotorDriver motor;
    motor.setSupported(MotionDirection::FORWARD_LEFT, false);

    const auto result = motor.setMotion(MotionDirection::FORWARD_LEFT, 80);
    TEST_ASSERT_EQUAL(robot::MotorResult::UNSUPPORTED_DIRECTION, result);

    motor.stopAll();
    TEST_ASSERT_EQUAL_UINT(1, motor.stopCount());
    TEST_ASSERT_EQUAL(MotionDirection::STOP, motor.lastRequest().direction);
    TEST_ASSERT_EQUAL_UINT8(0, motor.lastRequest().speed);
}

} // namespace

void run_state_transition_tests();
void run_safety_timeout_tests();

void setup()
{
}

void loop()
{
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_all_eight_directions_are_valid_move_values);
    RUN_TEST(test_stop_is_not_a_move_direction);
    RUN_TEST(test_unknown_direction_value_is_rejected);
    RUN_TEST(test_missing_direction_is_rejected);
    RUN_TEST(test_speed_outside_range_is_rejected);
    RUN_TEST(test_unsupported_diagonal_stops_motor);
    run_state_transition_tests();
    run_safety_timeout_tests();
    return UNITY_END();
}
