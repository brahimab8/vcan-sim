#include "csv_logger.h"

#include <iomanip>
#include <iostream>

CsvLogger::CsvLogger(const std::filesystem::path& dir)
    : dir_(dir)
{
    std::error_code ec;
    std::filesystem::create_directories(dir_, ec);
    if (ec) {
        std::cerr << "CsvLogger: failed to create directory " << dir_ << " : " << ec.message() << "\n";
    }
}

CsvLogger::~CsvLogger()
{
    for (auto &p : files_) {
        p.second.close();
    }
}

void CsvLogger::writeRow(const std::string& message_name, const std::vector<std::string>& columns, const std::vector<std::string>& values)
{
    if (columns.size() != values.size()) return;
    auto& f = files_[message_name]; // creates entry if needed

    // CSV field escaper: doubles quotes and wraps when needed
    auto escape_csv = [](const std::string& s) -> std::string {
        std::string out;
        bool need_quote = false;
        out.reserve(s.size());
        for (char c : s) {
            if (c == ',' || c == '"') need_quote = true;
            if (c == '"') out += "\"\""; else out += c;
        }
        if (need_quote) return '"' + out + '"';
        return out;
    };

    // If file is not open yet -> first time setup
    if (!f.is_open()) {
        std::filesystem::path path = dir_ / (message_name + ".csv");

        f.open(path, std::ios::out | std::ios::trunc);
        if (!f.is_open()) {
            std::cerr << "CsvLogger: failed to open " << path << " for writing\n";
            files_.erase(message_name);
            return;
        }

        // write header once
        for (size_t i = 0; i < columns.size(); ++i) {
            if (i > 0) f << ',';
            f << escape_csv(columns[i]);
        }
        f << '\n';
    }

    // write row
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) f << ',';
        f << escape_csv(values[i]);
    }
    f << '\n';
    f.flush();
}
