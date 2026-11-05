#include <array>
#include <cstdint>
#include <string>

#include <unity.h>

#include "robot_types.h"

namespace
{

using robot::CommandKind;
using robot::ControlCommand;
using robot::MotionDirection;
using robot::RobotMode;

struct ContractResponse
{
    int httpStatus = 400;
    bool accepted = false;
    const char* error = "INVALID_COMMAND";
    ControlCommand command{};
};

constexpr std::array<MotionDirection, 8> kDirections = {
    MotionDirection::FORWARD,
    MotionDirection::BACKWARD,
    MotionDirection::LEFT,
    MotionDirection::RIGHT,
    MotionDirection::FORWARD_LEFT,
    MotionDirection::FORWARD_RIGHT,
    MotionDirection::BACKWARD_LEFT,
    MotionDirection::BACKWARD_RIGHT};

ContractResponse parseLegacyCommand(const std::string& value)
{
    if (value.size() != 1)
    {
        return {};
    }

    ControlCommand command;
    command.hasDirection = value != "S";
    command.hasSpeed = value != "S";
    command.speed = 80;
    command.kind = value == "S" ? CommandKind::STOP : CommandKind::MOVE;

    if (value == "F")
    {
        command.direction = MotionDirection::FORWARD;
    }
    else if (value == "B")
    {
        command.direction = MotionDirection::BACKWARD;
    }
    else if (value == "L")
    {
        command.direction = MotionDirection::LEFT;
    }
    else if (value == "R")
    {
        command.direction = MotionDirection::RIGHT;
    }
    else if (value != "S")
    {
        return {};
    }

    return {200, true, nullptr, command};
}

ContractResponse parseVersionedCommand(int schemaVersion,
                                       const std::string& commandName,
                                       const std::string& direction,
                                       int speed)
{
    if (schemaVersion != 1 || commandName != "MOVE" || speed < 0
        || speed > 100)
    {
        return {};
    }

    const std::array<std::pair<const char*, MotionDirection>, 8> names = {{
        {"FORWARD", MotionDirection::FORWARD},
        {"BACKWARD", MotionDirection::BACKWARD},
        {"LEFT", MotionDirection::LEFT},
        {"RIGHT", MotionDirection::RIGHT},
        {"FORWARD_LEFT", MotionDirection::FORWARD_LEFT},
        {"FORWARD_RIGHT", MotionDirection::FORWARD_RIGHT},
        {"BACKWARD_LEFT", MotionDirection::BACKWARD_LEFT},
        {"BACKWARD_RIGHT", MotionDirection::BACKWARD_RIGHT}}};

    for (const auto& entry : names)
    {
        if (direction == entry.first)
        {
            ControlCommand parsed;
            parsed.kind = CommandKind::MOVE;
            parsed.direction = entry.second;
            parsed.speed = static_cast<std::uint8_t>(speed);
            parsed.hasDirection = true;
            parsed.hasSpeed = true;
            return {200, true, nullptr, parsed};
        }
    }

    return {};
}

ContractResponse rejectUnsupportedDirection(const ControlCommand& command)
{
    ContractResponse response;
    response.httpStatus = 409;
    response.error = "UNSUPPORTED_DIRECTION";
    response.command = command;
    return response;
}

void test_legacy_commands_accept_only_f_b_l_r_s()
{
    const std::array<std::string, 5> valid = {"F", "B", "L", "R", "S"};
    for (const auto& value : valid)
    {
        const ContractResponse response = parseLegacyCommand(value);
        TEST_ASSERT_EQUAL(200, response.httpStatus);
        TEST_ASSERT_TRUE(response.accepted);
    }

    const std::array<std::string, 5> invalid = {"", "X", "FF", "f", " F"};
    for (const auto& value : invalid)
    {
        const ContractResponse response = parseLegacyCommand(value);
        TEST_ASSERT_EQUAL(400, response.httpStatus);
        TEST_ASSERT_FALSE(response.accepted);
        TEST_ASSERT_EQUAL_STRING("INVALID_COMMAND", response.error);
    }
}

void test_legacy_mapping_preserves_domain_identity()
{
    TEST_ASSERT_EQUAL(MotionDirection::FORWARD,
                      parseLegacyCommand("F").command.direction);
    TEST_ASSERT_EQUAL(MotionDirection::BACKWARD,
                      parseLegacyCommand("B").command.direction);
    TEST_ASSERT_EQUAL(MotionDirection::LEFT,
                      parseLegacyCommand("L").command.direction);
    TEST_ASSERT_EQUAL(MotionDirection::RIGHT,
                      parseLegacyCommand("R").command.direction);
    TEST_ASSERT_EQUAL(CommandKind::STOP,
                      parseLegacyCommand("S").command.kind);
}

void test_versioned_command_accepts_all_eight_directions()
{
    const std::array<const char*, 8> names = {
        "FORWARD",
        "BACKWARD",
        "LEFT",
        "RIGHT",
        "FORWARD_LEFT",
        "FORWARD_RIGHT",
        "BACKWARD_LEFT",
        "BACKWARD_RIGHT"};

    for (std::size_t index = 0; index < names.size(); ++index)
    {
        const ContractResponse response =
            parseVersionedCommand(1, "MOVE", names[index], 80);
        TEST_ASSERT_EQUAL(200, response.httpStatus);
        TEST_ASSERT_TRUE(response.accepted);
        TEST_ASSERT_EQUAL(kDirections[index], response.command.direction);
        TEST_ASSERT_EQUAL_UINT8(80, response.command.speed);
    }
}

void test_versioned_command_rejects_invalid_schema_and_values()
{
    const std::array<ContractResponse, 4> responses = {
        parseVersionedCommand(2, "MOVE", "FORWARD", 80),
        parseVersionedCommand(1, "TURN", "FORWARD", 80),
        parseVersionedCommand(1, "MOVE", "NORTH", 80),
        parseVersionedCommand(1, "MOVE", "FORWARD", 101)};

    for (const ContractResponse& response : responses)
    {
        TEST_ASSERT_EQUAL(400, response.httpStatus);
        TEST_ASSERT_FALSE(response.accepted);
        TEST_ASSERT_EQUAL_STRING("INVALID_COMMAND", response.error);
    }
}

void test_unsupported_direction_returns_conflict_without_acceptance()
{
    const ContractResponse parsed =
        parseVersionedCommand(1, "MOVE", "FORWARD_LEFT", 80);
    const ContractResponse response =
        rejectUnsupportedDirection(parsed.command);

    TEST_ASSERT_EQUAL(409, response.httpStatus);
    TEST_ASSERT_FALSE(response.accepted);
    TEST_ASSERT_EQUAL_STRING("UNSUPPORTED_DIRECTION", response.error);
    TEST_ASSERT_EQUAL(MotionDirection::FORWARD_LEFT,
                      response.command.direction);
}

} // namespace

void setup()
{
}

void loop()
{
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_legacy_commands_accept_only_f_b_l_r_s);
    RUN_TEST(test_legacy_mapping_preserves_domain_identity);
    RUN_TEST(test_versioned_command_accepts_all_eight_directions);
    RUN_TEST(test_versioned_command_rejects_invalid_schema_and_values);
    RUN_TEST(test_unsupported_direction_returns_conflict_without_acceptance);
    return UNITY_END();
}
