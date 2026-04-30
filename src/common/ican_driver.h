#pragma once

#include "can_frame.h"

// Abstract CAN driver interface
// ECUs depend only on this interface, not on any platform-specific implementation
// Concrete drivers implement this interface for each target platform
class ICanDriver {
public:
    // Virtual destructor for proper cleanup in derived classes
    virtual ~ICanDriver() = default;

    // Send a CAN frame onto the bus
    // Returns true on success, false on failure
    virtual bool send(const CanFrame& frame) = 0;

    // Receive a CAN frame from the bus (blocking)
    // Returns true on success, false on error or timeout
    virtual bool receive(CanFrame& frame) = 0;
};