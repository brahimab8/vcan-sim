#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "isensors.h"

// Simulation wheel speed sensor that cycles through a fixed speed profile.
// Instantiate one per wheel with its specific profile.
class SimWheelSensor final : public IWheelSensor {
public:
    explicit SimWheelSensor(std::array<uint16_t, 8> profile)
        : profile_(profile) {}

    uint16_t readSpeed() noexcept override
    {
        const uint16_t value = profile_[index_];
        index_ = (index_ + 1) % profile_.size();
        return value;
    }

private:
    std::array<uint16_t, 8> profile_;
    std::size_t  index_{0};
};