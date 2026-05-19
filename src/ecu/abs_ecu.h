#pragma once

#include <cstdint>

#include "base_ecu.h"
#include "isensors.h"

// ABS ECU simulator
// Transmits four wheel speeds on CAN ID 0x200 every 20 ms
// Signal encoding follows vcansim.dbc definitions
class AbsEcu final : public BaseEcu {
public:
    explicit AbsEcu(
        ICanDriver& driver,
        ITimer& timer,
        IWheelSensor& front_left_sensor,
        IWheelSensor& front_right_sensor,
        IWheelSensor& rear_left_sensor,
        IWheelSensor& rear_right_sensor);

    // Run the cyclic transmit loop. Blocks until stop() is called
    void run() override;

    // Encode and send one ABSStatus frame.
    // Called by run() each cycle. Also callable directly in unit tests.
    void tick();

private:
    static constexpr uint32_t CYCLE_MS = 20;

    IWheelSensor& front_left_sensor_;
    IWheelSensor& front_right_sensor_;
    IWheelSensor& rear_left_sensor_;
    IWheelSensor& rear_right_sensor_;
};