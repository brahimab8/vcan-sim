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
    
    // Instantiate simulated wheel sensors (IDs used only for identifying sensors).
    SimWheelSensor front_left(0);
    SimWheelSensor front_right(1);
    SimWheelSensor rear_left(2);
    SimWheelSensor rear_right(3);

    // Construct the ABS ECU with injected platform dependencies and sensors.
    AbsEcu ecu(driver, timer, front_left, front_right, rear_left, rear_right);

    // Install Unix signal handlers so the runner can stop the ECU cleanly
    vcan_sim::runner::installSignalHandlers(ecu);

    // Run the ECU process main loop; this blocks until the ECU stops and
    // returns the process exit code.
    return vcan_sim::runner::runEcuProcess("abs_ecu", ecu);

}
