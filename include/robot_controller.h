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
    static constexpr Timestamp kDefaultCommandTimeout = 100;

    explicit RobotController(
        MotorDriver& motor,
        Timestamp commandTimeout = kDefaultCommandTimeout);

    // Validate and apply one command at the time it was received.
    CommandResult handleCommand(const ControlCommand& command,
                                Timestamp receivedAt);

    // Stop stale motion using a monotonic control-loop timestamp.
    void tick(Timestamp now);

    // Stop active motion when the controlling client is no longer connected.
    void onClientDisconnected();

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
    void stopForSafety(SafetyReason reason);

    MotorDriver& motor_;
    Timestamp commandTimeout_;
    RobotState state_{};
};

} // namespace robot

#endif // ROBOT_CONTROLLER_H
