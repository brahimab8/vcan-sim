#pragma once

#include <cstdint>

// Abstract timer interface
// Decouples ECU loop timing from any OS or hardware-specific implementation
class ITimer {
public:
    virtual ~ITimer() = default;

    // Block execution for the given number of milliseconds
    virtual void sleepMs(uint32_t ms) = 0;
};