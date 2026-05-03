#pragma once

#include <array>
#include <cstddef>

#include "isensor.h"

// Simulation RPM sensor that cycles through a fixed profile.
// Used for unit tests and simulation builds.
class SimRpmSensor final : public RpmSensor {
public:
    SimRpmSensor() = default;

    uint16_t read() noexcept override
    {
        const uint16_t value = RPM_PROFILE[index_];
        index_ = (index_ + 1) % RPM_PROFILE.size();
        return value;
    }

private:
    static constexpr std::array<uint16_t, 8> RPM_PROFILE = {
        800, 1200, 2000, 3500, 5000, 6500, 4000, 2500
    };
    std::size_t index_{0};
};
