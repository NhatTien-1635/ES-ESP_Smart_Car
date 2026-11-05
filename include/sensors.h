#ifndef SENSORS_H
#define SENSORS_H

#include <array>
#include <cstdint>

#include "robot_types.h"

namespace robot
{

enum class SensorValidity : std::uint8_t
{
    VALID,
    MISSING,
    STALE,
    INVALID
};

enum class SensorError : std::uint8_t
{
    NONE,
    NOT_INITIALIZED,
    READ_FAILED,
    OUT_OF_RANGE,
    STALE_SAMPLE
};

struct SensorReading
{
    std::array<bool, kLineSensorCount> line{};
    float distanceCm = 0.0F;
    float temperatureC = 0.0F;
    float humidityPercent = 0.0F;
    float batteryV = 0.0F;
    Timestamp sampledAt = 0;

    SensorValidity lineValidity = SensorValidity::MISSING;
    SensorValidity distanceValidity = SensorValidity::MISSING;
    SensorValidity temperatureValidity = SensorValidity::MISSING;
    SensorValidity humidityValidity = SensorValidity::MISSING;
    SensorValidity batteryValidity = SensorValidity::MISSING;

    SensorError lineError = SensorError::NOT_INITIALIZED;
    SensorError distanceError = SensorError::NOT_INITIALIZED;
    SensorError temperatureError = SensorError::NOT_INITIALIZED;
    SensorError humidityError = SensorError::NOT_INITIALIZED;
    SensorError batteryError = SensorError::NOT_INITIALIZED;
};

enum class SensorUpdateResult : std::uint8_t
{
    UPDATED,
    PARTIAL,
    FAILED
};

class SensorAdapter
{
public:
    virtual ~SensorAdapter() = default;

    // Sample hardware on a scheduler-owned cadence, never per HTTP request.
    virtual SensorUpdateResult update(Timestamp now) = 0;

    // Copy the latest reading so callers can serialize outside hardware locks.
    virtual bool copyReading(SensorReading& destination) const = 0;
};

} // namespace robot

#endif // SENSORS_H
