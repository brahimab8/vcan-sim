#pragma once

#include <cstdint>

#include "base_ecu.h"
#include "isensor.h"

// Motor ECU simulator
// Sends RPM and engine temperature on CAN ID 0x100 every 100 ms.
// Encoding follows the project's DBC/signal-encoding conventions.
class MotorEcu final : public BaseEcu {
public:
    explicit MotorEcu(ICanDriver& driver, ITimer& timer, RpmSensor& rpm_sensor, TempSensor& temp_sensor);

    // Run the cyclic transmit loop. Blocks until stop() is called
    void run() override;
    void tick();

    
private:
    static constexpr uint32_t CYCLE_MS  = 100;

    RpmSensor&  rpm_sensor_;
    TempSensor& temp_sensor_;
};