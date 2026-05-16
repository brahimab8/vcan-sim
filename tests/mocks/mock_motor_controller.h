#pragma once

#include <cstdint>
#include <vector>

#include "imotor_controller.h"

class MockMotorController final : public IMotorController {
public:
    void setTargetRpm(uint16_t rpm) noexcept override
    {
        calls_.push_back(rpm);
    }

    std::size_t callCount() const { return calls_.size(); }
    const std::vector<uint16_t>& calls() const { return calls_; }

private:
    std::vector<uint16_t> calls_;
};