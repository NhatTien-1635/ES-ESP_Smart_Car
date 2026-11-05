#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <cstdint>

#include "robot_types.h"
#include "sensors.h"

namespace robot
{

enum class ConnectionStatus : std::uint8_t
{
    CONNECTED,
    DISCONNECTED,
    STALE
};

struct ConnectionState
{
    ConnectionStatus status = ConnectionStatus::DISCONNECTED;
    Timestamp lastClientAt = 0;
    bool hasLastClientAt = false;
};

struct TelemetrySnapshot
{
    std::uint32_t schemaVersion = 1;
    RobotState state{};
    SensorReading sensors{};
    ConnectionState connection{};
};

class TelemetryProvider
{
public:
    virtual ~TelemetryProvider() = default;

    // Copy a coherent snapshot before JSON serialization or network I/O.
    virtual bool copySnapshot(TelemetrySnapshot& destination) const = 0;
};

} // namespace robot

#endif // TELEMETRY_H
