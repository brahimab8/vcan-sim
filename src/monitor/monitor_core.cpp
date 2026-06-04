#include "monitor_core.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <utility>

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

DecodedMessage MonitorCore::decode(const CanFrame& frame) const
{
    return decodeFrameWithDbc(frame);
}

void MonitorCore::logFrame(const CanFrame& frame, const DecodedMessage& decoded)
{
    std::string message_name = decoded.name.empty() ? ("MSG_" + toHexId(frame.id)) : decoded.name;

    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::string timestamp = std::to_string(now_ms);

    if (logger_) {
        std::vector<std::string> columns = {"timestamp", "frame_id"};
        std::vector<std::string> values = {timestamp, toHexId(frame.id)};

        if (!decoded.name.empty()) {
            columns.insert(columns.end(), decoded.columns.begin(), decoded.columns.end());
            values.insert(values.end(), decoded.values.begin(), decoded.values.end());
        } else {
            columns.push_back("data_hex");
            values.push_back(toDataHex(frame));
        }
        logger_->writeRow(message_name, columns, values);
    }
}

std::string MonitorCore::formatSummary(const CanFrame& frame, const DecodedMessage& decoded) const
{
    std::string message_name = decoded.name.empty() ? ("MSG_" + toHexId(frame.id)) : decoded.name;

    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::string timestamp = std::to_string(now_ms);

    std::ostringstream summary;
    summary << timestamp << " id=" << toHexId(frame.id)
            << " dlc=" << static_cast<int>(frame.dlc)
            << " msg=" << message_name;
    return summary.str();
}

std::string MonitorCore::processFrame(const CanFrame& frame)
{
    auto decoded = decode(frame);
    logFrame(frame, decoded);
    return formatSummary(frame, decoded);
}