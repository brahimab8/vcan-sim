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

    // Encode and send one motor frame immediately
    // Called by run() each cycle. Also callable directly in unit tests
    void tick();

    // Scaling helpers (public for unit testing)
    static uint16_t rpmToRaw(uint16_t rpm);
    static uint8_t  tempToRaw(int16_t temp);
    
private:
    static constexpr uint32_t CAN_ID    = 0x100;
    static constexpr uint8_t  FRAME_DLC = 3;
    static constexpr uint32_t CYCLE_MS  = 100;
    static constexpr uint16_t RPM_SCALE_FACTOR = 2U;   // scale=0.5 -> raw = rpm * 2
    static constexpr int16_t  TEMP_OFFSET    = 40;     // offset=-40 -> raw = temp + 40

    RpmSensor&  rpm_sensor_;
    TempSensor& temp_sensor_;
};