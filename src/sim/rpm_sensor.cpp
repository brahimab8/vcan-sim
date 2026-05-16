#include "rpm_sensor.h"

#include <algorithm>

uint16_t SimRpmSensor::read() noexcept
{
    if (current_ < target_)
        current_ = std::min<uint16_t>(current_ + kStep, target_);
    else if (current_ > target_)
        current_ = std::max<uint16_t>(current_ - kStep, target_);
    return current_;
}

void SimRpmSensor::setTarget(uint16_t target) noexcept
{
    target_ = target;
}