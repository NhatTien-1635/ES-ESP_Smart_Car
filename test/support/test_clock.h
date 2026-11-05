#ifndef TEST_CLOCK_H
#define TEST_CLOCK_H

#include "robot_types.h"

namespace robot::test
{

class TestClock
{
public:
    explicit TestClock(Timestamp initial = 0)
        : now_(initial)
    {
    }

    Timestamp now() const
    {
        return now_;
    }

    void set(Timestamp timestamp)
    {
        now_ = timestamp;
    }

    void advance(Timestamp amount)
    {
        now_ += amount;
    }

private:
    Timestamp now_;
};

} // namespace robot::test

#endif // TEST_CLOCK_H
