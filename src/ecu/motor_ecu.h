#pragma once

#include <cstdint>

#include "base_ecu.h"
#include "isensors.h"
#include "imotor_controller.h"

// Motor ECU simulator
// Sends RPM and engine temperature on CAN ID 0x100 every 100 ms.
// Receives target RPM commands on CAN ID 0x300 and forwards them to the motor controller
class MotorEcu final : public BaseEcu {
public:
    explicit MotorEcu(ICanDriver& driver, ITimer& timer,
                      IRpmSensor& rpm_sensor, ITempSensor& temp_sensor,
                      IMotorController& motor_controller);

    // Run the cyclic transmit loop. Blocks until stop() is called
    void run() override;

    // Encode and send one MotorStatus frame, then check for an incoming MotorControl command.
    // Called by run() each cycle. Also callable directly in unit tests.
    void tick();

private:
    static constexpr uint32_t CYCLE_MS  = 100;

    void handleCommand(const CanFrame& frame);

    IRpmSensor&  rpm_sensor_;
    ITempSensor& temp_sensor_;
    IMotorController& motor_controller_;
};