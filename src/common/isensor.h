#pragma once

#include <cstdint>

// Abstract sensor interface for reading physical quantities.
// Template parameter T is the value type (e.g., uint16_t for rpm, int16_t for temp).
//
// The read() method is non-blocking and returns the latest sampled value.
//
template<typename T>
class ISensor {
public:
    virtual ~ISensor() = default;

    // Read the latest sensor value. Non-blocking, returns POD only.
    virtual T read() noexcept = 0;
};

// Type aliases for specific sensors used in VcanSim.
// All values are in their encoded units.

using RpmSensor = ISensor<uint16_t>;        // Engine RPM (raw units: rpm)
using TempSensor = ISensor<int16_t>;        // Engine temperature (raw units: °C)
using WheelSensor = ISensor<uint16_t>;      // Wheel speed (raw units: deci-km/h, i.e., 0.1 km/h)
