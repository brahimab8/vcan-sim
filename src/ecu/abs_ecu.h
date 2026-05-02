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

    // Encode and send one ABS frame immediately
    // Called by run() each cycle. Also callable directly in unit tests
    void tick();

    // Scaling helper (public for unit testing)
    // speed in 0.1 km/h units (deci-km/h) -> raw uint16 value
    static uint16_t speedToRaw(uint16_t speed_deci_kmh);

private:
    static constexpr uint32_t CAN_ID             = 0x200;
    static constexpr uint8_t  FRAME_DLC          = 8;
    static constexpr uint32_t CYCLE_MS           = 20;
    static constexpr uint16_t MAX_WHEEL_RAW      = 3000; // DBC max 300.0 km/h

    WheelSensor& front_left_sensor_;
    WheelSensor& front_right_sensor_;
    WheelSensor& rear_left_sensor_;
    WheelSensor& rear_right_sensor_;
};