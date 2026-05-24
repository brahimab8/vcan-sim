#include "abs_ecu.h"
#include "wheel_sensor.h"
#include "ecu_runner.h"
#include "socketcan/driver.h"
#include "timer.h"

int main()
{
    // Create platform-specific interfaces: SocketCAN driver and a Linux timer.
    SocketCanDriver driver("vcan0");
    LinuxTimer timer;

    // Instantiate simulated wheel sensors with predefined speed profiles.
    SimWheelSensor front_left ({0, 100, 300, 600, 1000, 1200, 800, 400});
    SimWheelSensor front_right({0, 102, 303, 605, 1002, 1204, 802, 401});
    SimWheelSensor rear_left  ({0,  98, 297, 595,  998, 1196, 798, 399});
    SimWheelSensor rear_right ({0,  99, 298, 598,  999, 1198, 799, 398});

    // Construct the ABS ECU with injected platform dependencies and sensors.
    AbsEcu ecu(driver, timer, front_left, front_right, rear_left, rear_right);

    // Install Unix signal handlers so the runner can stop the ECU cleanly
    vcan_sim::runner::installSignalHandlers(ecu);
    
    // Run the ECU process main loop; this blocks until the ECU stops and
    // returns the process exit code.
    return vcan_sim::runner::runEcuProcess("abs_ecu", ecu);

}
