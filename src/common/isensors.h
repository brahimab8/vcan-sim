#pragma once

#include <cstdint>

// Named sensor interfaces for VcanSim.
// Separate interfaces (rather than ISensor<T>) allow one class to implement
// multiple sensor types without method name conflicts.

class IRpmSensor {
public:
    virtual ~IRpmSensor() = default;
    virtual uint16_t readRpm() noexcept = 0;
};

class ITempSensor {
public:
    virtual ~ITempSensor() = default;
    virtual int16_t readTemp() noexcept = 0;
};

class IWheelSensor {
public:
    virtual ~IWheelSensor() = default;
    virtual uint16_t readSpeed() noexcept = 0;
};