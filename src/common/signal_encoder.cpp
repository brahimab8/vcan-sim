#include "signal_encoder.h"

namespace {
    bool hasBytes(const uint8_t offset, const uint8_t width)
    {
        return static_cast<uint16_t>(offset) + static_cast<uint16_t>(width) <= CAN_MAX_DLC;
    }
}

namespace SignalEncoder {

    bool encodeUint16LE(CanFrame& frame, uint8_t offset, uint16_t value)
    {
        if (!hasBytes(offset, 2)) {
            return false;
        }

        frame.data[offset]     = static_cast<uint8_t>(value & 0xFF);        // LSB
        frame.data[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF); // MSB
        return true;
    }

    bool encodeUint8(CanFrame& frame, uint8_t offset, uint8_t value)
    {
        if (!hasBytes(offset, 1)) {
            return false;
        }

        frame.data[offset] = value;
        return true;
    }

    bool decodeUint16LE(const CanFrame& frame, uint8_t offset, uint16_t& value)
    {
        if (!hasBytes(offset, 2)) {
            value = 0;
            return false;
        }

        value = static_cast<uint16_t>(frame.data[offset]) |
                static_cast<uint16_t>(static_cast<uint16_t>(frame.data[offset + 1]) << 8);
        return true;
    }

    bool decodeUint8(const CanFrame& frame, uint8_t offset, uint8_t& value)
    {
        if (!hasBytes(offset, 1)) {
            value = 0;
            return false;
        }

        value = frame.data[offset];
        return true;
    }

} // namespace SignalEncoder