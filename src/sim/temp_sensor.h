#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "isensor.h"

// Simulation temperature sensor that cycles through a fixed profile.
// Used for unit tests and simulation builds.
class SimTempSensor final : public TempSensor {
public:
    SimTempSensor() = default;

    int16_t read() noexcept override
    {
        const int16_t value = TEMP_PROFILE[index_];
        index_ = (index_ + 1) % TEMP_PROFILE.size();
        return value;
    }

private:
    static constexpr std::array<int16_t, 8> TEMP_PROFILE = {
        20, 45, 70, 85, 90, 92, 90, 88
    };
    std::size_t index_{0};
};
