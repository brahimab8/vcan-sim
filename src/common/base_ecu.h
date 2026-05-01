#pragma once

#include "ican_driver.h"
#include "itimer.h"

// Abstract base class for all ECU simulators
// Holds the shared CAN driver reference, timer, and lifecycle control
// Concrete ECUs inherit from this and implement run()
class BaseEcu {
public:
    // The driver and timer must outlive the ECU instance.
    // BaseEcu does not own either, ownership remains with the caller.
    explicit BaseEcu(ICanDriver& driver, ITimer& timer);
    virtual ~BaseEcu() = default;

    // Disable copy semantics:
    // Copying would duplicate an ECU instance that refers to the same CAN driver,
    // which is not meaningful and may lead to unintended shared usage.
    BaseEcu(const BaseEcu&)            = delete;
    BaseEcu& operator=(const BaseEcu&) = delete;

    // Disable move semantics:
    // Moving does not transfer ownership of the driver (it is a reference),
    // and would effectively behave like a copy.
    BaseEcu(BaseEcu&&)                 = delete;
    BaseEcu& operator=(BaseEcu&&)      = delete;

    // Start the ECU transmit loop. Blocks until stop() is called.
    // Implementations must call start() before entering the loop.
    // Periodically check `running_` to exit cleanly.
    virtual void run() = 0;

    // Signal the run loop to stop
    void stop();

protected:
    // Set running_ to true. Call at the start of run() in derived classes
    void start();

    ICanDriver& driver_;      // CAN driver. Injected, not owned.
    ITimer&     timer_;       // Timer. Injected, not owned.
    bool running_{false};     // Loop control flag. Single-threaded use only.
                              // For thread safety, replace with std::atomic<bool>.
};