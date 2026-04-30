#pragma once

#include "can_frame.h"

// Helpers to pack/unpack little-endian integers into a CAN frame payload.
// All functions return `true` on success and perform bounds checks using `CAN_MAX_DLC`.
// Note: multi-byte operations are not atomic; callers must synchronize if needed.
namespace SignalEncoder {
    bool encodeUint16LE(CanFrame& frame, uint8_t offset, uint16_t value);
    bool encodeUint8(CanFrame& frame, uint8_t offset, uint8_t value);
    bool decodeUint16LE(const CanFrame& frame, uint8_t offset, uint16_t& value);
    bool decodeUint8(const CanFrame& frame, uint8_t offset, uint8_t& value);
} // namespace SignalEncoder