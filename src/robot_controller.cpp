#include "robot_controller.h"

namespace robot
{

RobotController::RobotController(MotorDriver& motor,
                                 Timestamp commandTimeout)
    : motor_(motor),
      commandTimeout_(commandTimeout)
{
}

CommandResult RobotController::handleCommand(const ControlCommand& command,
                                             Timestamp receivedAt)
{
    switch (command.kind)
    {
    case CommandKind::MOVE:
        return applyMove(command, receivedAt);
    case CommandKind::STOP:
        return applyStop(receivedAt);
    case CommandKind::SET_MODE:
        if (!command.hasMode || !isValidMode(command.mode))
        {
            return reject(CommandStatus::INVALID);
        }
        return applyMode(command.mode, receivedAt);
    }

    return reject(CommandStatus::INVALID);
}

const RobotState& RobotController::state() const
{
    return state_;
}

void RobotController::tick(Timestamp now)
{
    if (state_.motion == MotionDirection::STOP
        || now < state_.lastCommandAt
        || now - state_.lastCommandAt < commandTimeout_)
    {
        return;
    }

    stopForSafety(SafetyReason::TIMEOUT);
}

void RobotController::onClientDisconnected()
{
    if (state_.motion != MotionDirection::STOP)
    {
        stopForSafety(SafetyReason::DISCONNECTED);
    }
}

bool RobotController::isMoveDirection(MotionDirection direction)
{
    switch (direction)
    {
    case MotionDirection::FORWARD:
    case MotionDirection::BACKWARD:
    case MotionDirection::LEFT:
    case MotionDirection::RIGHT:
    case MotionDirection::FORWARD_LEFT:
    case MotionDirection::FORWARD_RIGHT:
    case MotionDirection::BACKWARD_LEFT:
    case MotionDirection::BACKWARD_RIGHT:
        return true;
    case MotionDirection::STOP:
        return false;
    }

    return false;
}

bool RobotController::isValidMode(RobotMode mode)
{
    switch (mode)
    {
    case RobotMode::IDLE:
    case RobotMode::MANUAL:
    case RobotMode::LINE_TRACKING:
        return true;
    }

    return false;
}

bool RobotController::isValidSpeed(std::uint8_t speed)
{
    return speed >= kMinSpeedPercent && speed <= kMaxSpeedPercent;
}

CommandResult RobotController::reject(CommandStatus status) const
{
    return {status, state_.motion};
}

CommandResult RobotController::applyMove(const ControlCommand& command,
                                          Timestamp receivedAt)
{
    if (!command.hasDirection || !command.hasSpeed
        || !isMoveDirection(command.direction)
        || !isValidSpeed(command.speed))
    {
        return reject(CommandStatus::INVALID);
    }

    // Stop the previous request before replacing it with a new one.
    if (state_.motion != MotionDirection::STOP)
    {
        motor_.stopAll();
    }

    const MotorResult result =
        motor_.setMotion(command.direction, command.speed);
    if (result != MotorResult::APPLIED)
    {
        stopForSafety(SafetyReason::NONE);
        return reject(CommandStatus::UNSUPPORTED);
    }

    state_.mode = RobotMode::MANUAL;
    state_.motion = command.direction;
    state_.speed = command.speed;
    state_.lastCommandAt = receivedAt;
    state_.safetyReason = SafetyReason::NONE;
    return {CommandStatus::ACCEPTED, state_.motion};
}

CommandResult RobotController::applyMode(RobotMode mode,
                                          Timestamp receivedAt)
{
    motor_.stopAll();
    state_.mode = mode;
    state_.motion = MotionDirection::STOP;
    state_.speed = 0;
    state_.lastCommandAt = receivedAt;
    state_.safetyReason = SafetyReason::NONE;
    return {CommandStatus::ACCEPTED, state_.motion};
}

CommandResult RobotController::applyStop(Timestamp receivedAt)
{
    motor_.stopAll();
    state_.mode = RobotMode::IDLE;
    state_.motion = MotionDirection::STOP;
    state_.speed = 0;
    state_.lastCommandAt = receivedAt;
    state_.safetyReason = SafetyReason::NONE;
    return {CommandStatus::ACCEPTED, state_.motion};
}

void RobotController::stopForSafety(SafetyReason reason)
{
    motor_.stopAll();
    state_.mode = RobotMode::IDLE;
    state_.motion = MotionDirection::STOP;
    state_.speed = 0;
    state_.safetyReason = reason;
}

} // namespace robot
