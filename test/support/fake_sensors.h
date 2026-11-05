#ifndef FAKE_SENSORS_H
#define FAKE_SENSORS_H

#include <cstddef>

#include "sensors.h"

namespace robot::test
{

class FakeSensors final : public SensorAdapter
{
public:
    SensorUpdateResult update(Timestamp now) override
    {
        lastUpdateAt_ = now;
        updateCount_++;
        return updateResult_;
    }

    bool copyReading(SensorReading& destination) const override
    {
        if (!copyAvailable_)
        {
            return false;
        }

        destination = reading_;
        return true;
    }

    void setReading(const SensorReading& reading)
    {
        reading_ = reading;
        copyAvailable_ = true;
    }

    void setUpdateResult(SensorUpdateResult result)
    {
        updateResult_ = result;
    }

    void setCopyAvailable(bool available)
    {
        copyAvailable_ = available;
    }

    std::size_t updateCount() const
    {
        return updateCount_;
    }

    Timestamp lastUpdateAt() const
    {
        return lastUpdateAt_;
    }

private:
    SensorReading reading_{};
    SensorUpdateResult updateResult_ = SensorUpdateResult::UPDATED;
    Timestamp lastUpdateAt_ = 0;
    std::size_t updateCount_ = 0;
    bool copyAvailable_ = true;
};

} // namespace robot::test

#endif // FAKE_SENSORS_H
