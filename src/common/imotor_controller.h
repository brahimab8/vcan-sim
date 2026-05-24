#pragma once

#include <cstdint>

// Interface for controlling the motor target RPM.
// Introduced for testability.
class IMotorController {
public:
    virtual ~IMotorController() = default;
    virtual void setTargetRpm(uint16_t rpm) noexcept = 0;
};