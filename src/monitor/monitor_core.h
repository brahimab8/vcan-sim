#pragma once

#include <optional>
#include <string>

#include "can_frame.h"
#include "csv_logger.h"
#include "decoder.h"

// MonitorCore decodes CAN frames, optionally logs decoded rows to CSV,
// and produces a short textual summary for console output.
class MonitorCore {
public:
    MonitorCore(std::optional<std::string> dbc_path, std::optional<std::string> csv_dir);

    // Decode a frame using the DBC-generated helpers.
    DecodedMessage decode(const CanFrame& frame) const;

    // Write a CSV row if logging is enabled.
    void logFrame(const CanFrame& frame, const DecodedMessage& decoded);

    // Return a compact summary string for console output.
    std::string formatSummary(const CanFrame& frame, const DecodedMessage& decoded) const;

    // Decode the frame, optionally write a CSV row, and return a compact summary.
    std::string processFrame(const CanFrame& frame);

private:
    static std::string toHexId(uint32_t id);
    static std::string toDataHex(const CanFrame& frame);

    std::optional<std::string> dbc_path_;
    std::optional<CsvLogger> logger_;
};