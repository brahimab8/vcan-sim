#pragma once

#include <string>

#include "ican_driver.h"

// Linux SocketCAN implementation of ICanDriver.
// Uses an AF_CAN raw socket bound to a named interface (for example: vcan0).
class SocketCanDriver final : public ICanDriver {
public:
    explicit SocketCanDriver(std::string interface_name);   
    ~SocketCanDriver() override;

    // Delete copy and move constructors and assignment operators to prevent
    // accidental copying or moving of the driver, which manages a unique socket resource.
    SocketCanDriver(const SocketCanDriver&) = delete;
    SocketCanDriver& operator=(const SocketCanDriver&) = delete;
    SocketCanDriver(SocketCanDriver&&) = delete;
    SocketCanDriver& operator=(SocketCanDriver&&) = delete;

    // ICanDriver interface implementation
    bool send(const CanFrame& frame) override;
    bool receive(CanFrame& frame) override;

private:
    int socket_fd_{-1};             // File descriptor for the CAN socket.
    std::string interface_name_;    // Name of the CAN interface
};
