#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include "motor_control_client.h"
#include "socketcan/driver.h"

namespace {

// Parse a NUL-terminated C string into a float using `std::strtof`.
// Returns true only when at least one character was parsed and the
// entire string was consumed (no trailing characters).
bool parseFloat(const char* text, float& value)
{
    char* end = nullptr;
    value = std::strtof(text, &end);
    return end != text && *end == '\0';
}

void printUsage(const char* program)
{
    // `program` is typically `argv[0]` (the program name/path as invoked).
    std::cerr << "Usage: " << program << " <target_rpm> [interface]\n"
              << "Example: " << program << " 3000 vcan0\n";
}

} // namespace

int main(int argc, char** argv)
{
    if (argc < 2 || argc > 3) {
        printUsage(argv[0]);
        return 1;
    }

    float target_rpm = 0.0f;
    if (!parseFloat(argv[1], target_rpm)) {
        std::cerr << "motor_control: invalid target_rpm: " << argv[1] << '\n';
        printUsage(argv[0]);
        return 1;
    }

    std::string interface = "vcan0";
    if (argc == 3) {
        interface = argv[2];
    }

    try {
        SocketCanDriver driver(interface);
        MotorControlClient client(driver);

        if (!client.sendTargetRpm(target_rpm)) {
            std::cerr << "motor_control: failed to send target RPM command\n";
            return 1;
        }

        std::cout << "motor_control: sent target_rpm=" << target_rpm
                  << " on " << interface << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "motor_control: " << e.what() << '\n';
        return 1;
    }
}
