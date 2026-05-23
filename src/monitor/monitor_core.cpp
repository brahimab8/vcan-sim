#include "monitor_core.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <utility>

#include "decoder.h"

// MonitorCore: decode a CAN frame, optionally log it, and return a summary.

MonitorCore::MonitorCore(std::optional<std::string> dbc_path, std::optional<std::string> csv_dir)
    : dbc_path_(std::move(dbc_path)), logger_(csv_dir ? std::make_optional<CsvLogger>(*csv_dir) : std::nullopt)
{
}

std::string MonitorCore::toHexId(uint32_t id)
{
    std::ostringstream ss;
    ss << "0x" << std::hex << std::uppercase << id;
    return ss.str();
}

std::string MonitorCore::toDataHex(const CanFrame& frame)
{
    std::ostringstream data_hex;
    for (unsigned i = 0; i < frame.dlc; ++i) {
        data_hex << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(frame.data[i]);
    }
    return data_hex.str();
}

std::string MonitorCore::processFrame(const CanFrame& frame)
{
    // Decode with DBC-generated helpers; unknown frames have empty name.
    auto decoded = decodeFrameWithDbc(frame);
    // Determine the message name: use the decoded name if available, otherwise generate a default.
    std::string message_name = decoded.name.empty() ? ("MSG_" + toHexId(frame.id)) : decoded.name;

    // Get the current timestamp in milliseconds since epoch.
    auto now = std::chrono::system_clock::now();
    // Convert to milliseconds.
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    // Format the timestamp as a string.
    std::string timestamp = std::to_string(now_ms);

    // If CSV logging requested, write a row (decoded values or raw hex).
    if (logger_) {
        // Start with common columns: timestamp and frame ID.
        std::vector<std::string> columns = {"timestamp", "frame_id"};
        std::vector<std::string> values = {timestamp, toHexId(frame.id)};

        // If we have decoded values, append them; otherwise log raw data hex.
        if (!decoded.name.empty()) {
            columns.insert(columns.end(), decoded.columns.begin(), decoded.columns.end());
            values.insert(values.end(), decoded.values.begin(), decoded.values.end());
        } else {
            columns.push_back("data_hex");
            values.push_back(toDataHex(frame));
        }
        // Write the row to the CSV logger.
        logger_->writeRow(message_name, columns, values);
    }

    // Build a compact console-friendly summary describing the frame.
    std::ostringstream summary;
    summary << timestamp << " id=" << toHexId(frame.id)
            << " dlc=" << static_cast<int>(frame.dlc)
            << " msg=" << message_name;
    return summary.str();
}