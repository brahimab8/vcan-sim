#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "isensor.h"

// Simulation wheel speed sensor that cycles through a fixed profile.
// One instance per wheel (FL, FR, RL, RR).
class SimWheelSensor final : public WheelSensor {
public:
    // Wheel index: 0=FL, 1=FR, 2=RL, 3=RR
    explicit SimWheelSensor(unsigned int wheel_index) : wheel_index_(wheel_index) {}

    uint16_t read() noexcept override
    {
        const uint16_t value = ALL_WHEELS[wheel_index_][index_];
        index_ = (index_ + 1) % ALL_WHEELS[wheel_index_].size();
        return value;
    }

private:
    static constexpr std::array<std::array<uint16_t, 8>, 4> ALL_WHEELS = {{
        // FL profile (deci-km/h)
        {0, 100, 300, 600, 1000, 1200, 800, 400},
        // FR profile
        {0, 102, 303, 605, 1002, 1204, 802, 401},
        // RL profile
        {0, 98, 297, 595, 998, 1196, 798, 399},
        // RR profile
        {0, 99, 298, 598, 999, 1198, 799, 398}
    }};

    unsigned int wheel_index_;
    std::size_t  index_{0};
};
