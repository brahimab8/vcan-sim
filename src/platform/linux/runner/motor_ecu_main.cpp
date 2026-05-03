#include "motor_ecu.h"
#include "rpm_sensor.h"
#include "ecu_runner.h"
#include "socketcan/driver.h"
#include "timer.h"
#include "temp_sensor.h"

int main()
{
    // Create platform-specific interfaces: SocketCAN driver and a Linux timer.
    SocketCanDriver driver("vcan0");
    LinuxTimer timer;

    // Create simulated sensors used by the Motor ECU
    SimRpmSensor rpm_sensor;
    SimTempSensor temp_sensor;

    // Inject dependencies into the ECU implementation
    MotorEcu ecu(driver, timer, rpm_sensor, temp_sensor);

    // Ensure the process responds to signals
    vcan_sim::runner::installSignalHandlers(ecu);

    // Start the ECU process main loop and return its exit status
    return vcan_sim::runner::runEcuProcess("motor_ecu", ecu);

}
