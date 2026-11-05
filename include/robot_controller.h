#ifndef ROBOT_CONTROLLER_H
#define ROBOT_CONTROLLER_H

#include <cstdint>

#include "motor_driver.h"
#include "robot_types.h"

namespace robot
{

class RobotController final
{
public:
    explicit RobotController(MotorDriver& motor);

    // Validate and apply one command at the time it was received.
    CommandResult handleCommand(const ControlCommand& command,
                                Timestamp receivedAt);

    const RobotState& state() const;

private:
    static bool isMoveDirection(MotionDirection direction);
    static bool isValidMode(RobotMode mode);
    static bool isValidSpeed(std::uint8_t speed);

    CommandResult reject(CommandStatus status) const;
    CommandResult applyMove(const ControlCommand& command,
                            Timestamp receivedAt);
    CommandResult applyMode(RobotMode mode, Timestamp receivedAt);
    CommandResult applyStop(Timestamp receivedAt);
    void clearMotion(SafetyReason reason);

    MotorDriver& motor_;
    RobotState state_{};
};

} // namespace robot

#endif // ROBOT_CONTROLLER_H
