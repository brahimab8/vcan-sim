#include "motor_ecu.h"
#include "vcansim.h" // Generated from vcansim.dbc by cantools

MotorEcu::MotorEcu(ICanDriver& driver, ITimer& timer,
                   IRpmSensor& rpm_sensor, ITempSensor& temp_sensor,
                   IMotorController& motor_controller)
    : BaseEcu(driver, timer)
    , rpm_sensor_(rpm_sensor)
    , temp_sensor_(temp_sensor)
    , motor_controller_(motor_controller)
{
}

void MotorEcu::run()
{
    start();
    while (running_) {
        tick();
        timer_.sleepMs(CYCLE_MS);
    }
}

void MotorEcu::tick()
{
    const uint16_t rpm  = rpm_sensor_.readRpm();
    const int16_t  temp = temp_sensor_.readTemp();

    vcansim_motor_status_t msg{};
    msg.rpm         = vcansim_motor_status_rpm_encode(static_cast<float>(rpm));
    msg.temperature = vcansim_motor_status_temperature_encode(static_cast<float>(temp));

    CanFrame frame{};
    frame.id  = VCANSIM_MOTOR_STATUS_FRAME_ID;
    frame.dlc = VCANSIM_MOTOR_STATUS_LENGTH;
    vcansim_motor_status_pack(frame.data.data(), &msg, frame.data.size());

    // Send the frame. If sending fails, skip this cycle and try again next time.
    if (!driver_.send(frame)) {
        // error handling deferred to future iterations.
    }

    // Drain pending frames so control commands are not delayed behind other bus traffic.
    CanFrame cmd{};
    while (driver_.receive(cmd)) {
        if (cmd.id == VCANSIM_MOTOR_CONTROL_FRAME_ID) {
            handleCommand(cmd);
        }
    }
}

void MotorEcu::handleCommand(const CanFrame& frame)
{
    vcansim_motor_control_t msg{};
    vcansim_motor_control_unpack(&msg, frame.data.data(), frame.dlc);

    const auto target = static_cast<uint16_t>(
        vcansim_motor_control_target_rpm_decode(msg.target_rpm));

    motor_controller_.setTargetRpm(target);
}