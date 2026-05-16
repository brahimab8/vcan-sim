#pragma once

#include <cstdint>
#include "isensor.h"

// Simulation RPM sensor that ramps toward a target RPM in fixed steps.
// Target can be updated externally via setTarget() (e.g. from a CAN command).
class SimRpmSensor final : public RpmSensor {
public:
    SimRpmSensor() = default;

    uint16_t read() noexcept override;
    void setTarget(uint16_t target) noexcept;

private:
    static constexpr uint16_t kStep    = 100u;
    static constexpr uint16_t kDefault = 800u;

    uint16_t current_{kDefault};
    uint16_t target_ {kDefault};
};