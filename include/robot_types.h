#ifndef ROBOT_TYPES_H
#define ROBOT_TYPES_H

#include <cstddef>
#include <cstdint>

namespace robot
{

constexpr std::uint8_t kMinSpeedPercent = 0;
constexpr std::uint8_t kMaxSpeedPercent = 100;
constexpr std::size_t kLineSensorCount = 5;

using Timestamp = std::uint64_t;

enum class RobotMode : std::uint8_t
{
    IDLE,
    MANUAL,
    LINE_TRACKING
};

enum class MotionDirection : std::uint8_t
{
    STOP,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    FORWARD_LEFT,
    FORWARD_RIGHT,
    BACKWARD_LEFT,
    BACKWARD_RIGHT
};

enum class CommandKind : std::uint8_t
{
    MOVE,
    STOP,
    SET_MODE
};

enum class CommandStatus : std::uint8_t
{
    ACCEPTED,
    INVALID,
    UNSUPPORTED
};

enum class SafetyReason : std::uint8_t
{
    NONE,
    TIMEOUT,
    DISCONNECTED,
    INVALID_SENSOR,
    OBSTACLE
};

struct ControlCommand
{
    CommandKind kind = CommandKind::STOP;
    MotionDirection direction = MotionDirection::STOP;
    std::uint8_t speed = 0;
    RobotMode mode = RobotMode::IDLE;
    Timestamp receivedAt = 0;
    bool hasDirection = false;
    bool hasSpeed = false;
    bool hasMode = false;
};

struct CommandResult
{
    CommandStatus status = CommandStatus::INVALID;
    MotionDirection motion = MotionDirection::STOP;
};

struct RobotState
{
    RobotMode mode = RobotMode::IDLE;
    MotionDirection motion = MotionDirection::STOP;
    std::uint8_t speed = 0;
    Timestamp lastCommandAt = 0;
    SafetyReason safetyReason = SafetyReason::NONE;
};

} // namespace robot

#endif // ROBOT_TYPES_H
