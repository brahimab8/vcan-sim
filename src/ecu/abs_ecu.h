#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "base_ecu.h"
#include "isensor.h"

// ABS ECU simulator
// Transmits four wheel speeds on CAN ID 0x200 every 20 ms
// Signal encoding follows vcansim.dbc definitions
class AbsEcu final : public BaseEcu {
public:
    explicit AbsEcu(
        ICanDriver& driver,
        ITimer& timer,
        WheelSensor& front_left_sensor,
        WheelSensor& front_right_sensor,
        WheelSensor& rear_left_sensor,
        WheelSensor& rear_right_sensor);

    // Run the cyclic transmit loop. Blocks until stop() is called
    void run() override;
    void tick();

private:
    static constexpr uint32_t CYCLE_MS = 20;

    WheelSensor& front_left_sensor_;
    WheelSensor& front_right_sensor_;
    WheelSensor& rear_left_sensor_;
    WheelSensor& rear_right_sensor_;
};