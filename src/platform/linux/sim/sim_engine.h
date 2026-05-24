#pragma once

#include <algorithm>
#include <cstdint>

#include "isensors.h"
#include "imotor_controller.h"

// Simulated engine for runner and integration builds.
// Implements IRpmSensor, ITempSensor, and IMotorController in one object.
// RPM ramps toward the target in fixed steps each readRpm().
// Temperature is fixed; can be made dynamic in a future iteration.
//
// Not part of the ECU layer. Constructed by the runner and injected
// into MotorEcu as three separate interface references.
class SimEngine final : public IRpmSensor,
                        public ITempSensor,
                        public IMotorController {
public:
    SimEngine() = default;

    uint16_t readRpm() noexcept override
    {
        if (current_rpm_ < target_rpm_)
            current_rpm_ = std::min<uint16_t>(current_rpm_ + kRpmStep, target_rpm_);
        else if (current_rpm_ > target_rpm_)
            current_rpm_ = std::max<uint16_t>(current_rpm_ - kRpmStep, target_rpm_);
        return current_rpm_;
    }

    int16_t readTemp() noexcept override
    {
        return temp_;
    }

    void setTargetRpm(uint16_t rpm) noexcept override
    {
        target_rpm_ = rpm;
    }

private:
    static constexpr uint16_t kRpmStep     = 100u;
    static constexpr uint16_t kDefaultRpm  = 800u;
    static constexpr int16_t  kDefaultTemp = 70;

    uint16_t current_rpm_{kDefaultRpm};
    uint16_t target_rpm_ {kDefaultRpm};
    int16_t  temp_       {kDefaultTemp};
};