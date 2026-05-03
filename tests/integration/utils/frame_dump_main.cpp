#include <cstdint>
#include <iomanip>   // Formatting tools for streams (hex, setw, setfill)
#include <iostream> 
#include <sstream>   // String stream (used to build strings like a buffer)
#include <string>

#include "abs_ecu.h"         
#include "motor_ecu.h"       

#include "mock_can_driver.h"
#include "mock_sensor.h"
#include "mock_timer.h"

namespace {

    // Converts CAN frame payload (bytes) into a hexadecimal string
    std::string payloadToHex(const CanFrame& frame)
    {
        // Create a string stream
        std::ostringstream oss;

        // Set formatting rules:
        // std::hex -> print numbers in base 16
        // std::setfill('0') -> pad missing digits with '0'
        oss << std::hex << std::setfill('0');

        // Loop over each byte in the CAN frame payload
        for (std::size_t i = 0; i < frame.dlc; ++i)
        {
            // std::setw(2) ensures each byte is printed as 2 hex digits
            // static_cast<unsigned> ensures the byte is treated as a number
            oss << std::setw(2)
                << static_cast<unsigned>(frame.data[i]);
        }

        // Convert the stream buffer into a string and return it
        return oss.str();
    }

    // Prints all CAN frames in a readable CSV-like format
    void dumpFrames(const std::string& topic, const std::vector<CanFrame>& frames)
    {
        // Loop through every CAN frame in the vector
        for (const auto& frame : frames)
        {
            // Print:
            // topic, frame ID, data length, payload in hex
            std::cout << topic << ','

                      // CAN message identifier
                      << frame.id << ','

                      // Data length code (number of bytes in payload)
                      << static_cast<unsigned>(frame.dlc) << ','

                      // Hex representation of payload
                      << payloadToHex(frame)

                      // End the line
                      << '\n';
        }
    }

} // end anonymous namespace
// Functions above are private to this file only

// Program entry point
int main()
{
    // -----------------------------
    // FIRST TEST: Motor ECU
    // -----------------------------
    {
        // Fake CAN driver to collect outgoing messages
        MockCanDriver driver;

        // Fake timer for ECU timing logic
        MockTimer timer;

        // Fake RPM sensor returning values between 800 and 1200
        MockSensor<uint16_t> rpm_sensor{{800, 1200}};

        // Fake engine temperature sensor returning 20 to 45 degrees
        MockSensor<int16_t> temp_sensor{{20, 45}};

        // Create Motor ECU with dependencies injected
        MotorEcu motor{driver, timer, rpm_sensor, temp_sensor};

        // Run ECU logic twice (simulate two cycles of operation)
        motor.tick();
        motor.tick();

        // Print all CAN frames sent by motor ECU
        dumpFrames("motor", driver.sentFrames());
    }

    // -----------------------------
    // SECOND TEST: ABS ECU
    // -----------------------------
    {
        // New independent fake CAN driver
        MockCanDriver driver;

        // New fake timer
        MockTimer timer;

        // Fake wheel speed sensors (front left, front right, rear left, rear right)
        MockSensor<uint16_t> fl{{0, 100}};
        MockSensor<uint16_t> fr{{0, 102}};
        MockSensor<uint16_t> rl{{0, 98}};
        MockSensor<uint16_t> rr{{0, 99}};

        // Create ABS ECU with sensor inputs
        AbsEcu abs{driver, timer, fl, fr, rl, rr};

        // Run ABS logic twice
        abs.tick();
        abs.tick();

        // Print all CAN frames sent by ABS ECU
        dumpFrames("abs", driver.sentFrames());
    }

    // Program finished successfully
    return 0;
}