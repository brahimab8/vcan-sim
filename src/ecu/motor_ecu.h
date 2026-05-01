#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "base_ecu.h"

// Motor ECU simulator
// Sends RPM and engine temperature on CAN ID 0x100 every 100 ms.
// Encoding follows the project's DBC/signal-encoding conventions.
class MotorEcu final : public BaseEcu {
public:
    explicit MotorEcu(ICanDriver& driver, ITimer& timer);

    // Run the cyclic transmit loop. Blocks until stop() is called
    void run() override;

    // Encode and send one motor frame immediately
    // Called by run() each cycle. Also callable directly in unit tests
    void tick();

    // Scaling helpers (public for unit testing)
    static uint16_t rpmToRaw(uint16_t rpm);
    static uint8_t  tempToRaw(int8_t temp);
    
private:
    // Cyclic RPM values used by `tick()` (in rpm).
    static constexpr std::array<uint16_t, 8> RPM_PROFILE = {
        800, 1200, 2000, 3500, 5000, 6500, 4000, 2500
    };

    // Cyclic temperature values used by `tick()` (°C).
    static constexpr std::array<int8_t, 8> TEMP_PROFILE = {
        20, 45, 70, 85, 90, 92, 90, 88
    };

    static constexpr uint32_t CAN_ID    = 0x100;
    static constexpr uint8_t  FRAME_DLC = 3;
    static constexpr uint32_t CYCLE_MS  = 100;
    static constexpr uint16_t RPM_SCALE_FACTOR = 2U;   // scale=0.5 -> raw = rpm * 2
    static constexpr int8_t   TEMP_OFFSET    = 40;     // offset=-40 -> raw = temp + 40

    std::size_t profile_index_{0};  // current position in the simulated profiles
};