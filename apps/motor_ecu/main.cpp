#include "motor_ecu.h"
#include "sim_engine.h"
#include "ecu_runner.h"
#include "socketcan/driver.h"
#include "timer.h"

int main()
{
    // Create platform-specific interfaces: SocketCAN driver and a Linux timer.
    SocketCanDriver driver("vcan0");
    LinuxTimer      timer;

    // SimEngine implements IRpmSensor, ITempSensor, and IMotorController.
    // It is the single simulated engine object injected into MotorEcu
    // as three separate interface references.
    SimEngine engine;

    // Inject dependencies into the ECU implementation
    MotorEcu ecu(driver, timer, engine, engine, engine);

    // Ensure the process responds to signals
    vcan_sim::runner::installSignalHandlers(ecu);

    // Start the ECU process main loop and return its exit status
    return vcan_sim::runner::runEcuProcess("motor_ecu", ecu);

}
