#pragma once

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

class CsvLogger {
public:
    explicit CsvLogger(const std::filesystem::path& dir);
    ~CsvLogger();

    // Write a row for a named message. The first call for a message writes a header.
    void writeRow(
        const std::string& message_name, 
        const std::vector<std::string>& columns, 
        const std::vector<std::string>& values
    );

private:
    std::filesystem::path dir_;
    std::map<std::string, std::ofstream> files_;
};
