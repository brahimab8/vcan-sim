#include "motor_ecu.h"
#include "vcansim.h" // Generated from vcansim.dbc by cantools


MotorEcu::MotorEcu(ICanDriver& driver, ITimer& timer, RpmSensor& rpm_sensor, TempSensor& temp_sensor)
    : BaseEcu(driver, timer)
    , rpm_sensor_(rpm_sensor)
    , temp_sensor_(temp_sensor)
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
    const uint16_t rpm  = rpm_sensor_.read();
    const int16_t  temp = temp_sensor_.read();

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
}