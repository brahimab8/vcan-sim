#pragma once

#include <cstdint>
#include <array>

// Classical CAN frames (CAN 2.0A / 2.0B) are limited to 8 bytes payload (ISO 11898-1)
// CAN FD supports up to 64 bytes (out of scope for this project)
constexpr uint8_t CAN_MAX_DLC = 8;

// Represents a single CAN frame (platform-independent)
struct CanFrame {
    uint32_t id;        // CAN identifier (11-bit standard or 29-bit extended)
    uint8_t  dlc;       // Data length code (0-8 bytes) indicating number of valid bytes in 'data'
    std::array<uint8_t, CAN_MAX_DLC> data;  // Raw payload bytes; Only the first 'dlc' bytes are valid
};