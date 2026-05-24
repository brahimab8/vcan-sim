#pragma once

#include <string>
#include <vector>
#include <map>

#include "can_frame.h"

struct DecodedMessage {
    std::string name;
    std::vector<std::string> columns;
    std::vector<std::string> values;
};

// Decode a raw CAN frame using the DBC-generated `vcansim` API when possible.
// Returns an empty name if the frame ID is unknown or decode failed.
DecodedMessage decodeFrameWithDbc(const CanFrame& frame) noexcept;
