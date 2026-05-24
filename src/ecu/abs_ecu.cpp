#include "abs_ecu.h"
#include "vcansim.h" // Generated from vcansim.dbc by cantools

AbsEcu::AbsEcu(
    ICanDriver& driver,
    ITimer& timer,
    IWheelSensor& front_left_sensor,
    IWheelSensor& front_right_sensor,
    IWheelSensor& rear_left_sensor,
    IWheelSensor& rear_right_sensor)
    : BaseEcu(driver, timer)
    , front_left_sensor_(front_left_sensor)
    , front_right_sensor_(front_right_sensor)
    , rear_left_sensor_(rear_left_sensor)
    , rear_right_sensor_(rear_right_sensor)
{
}

void AbsEcu::run()
{
    start();
    while (running_) {
        tick();
        timer_.sleepMs(CYCLE_MS);
    }
}

void AbsEcu::tick()
{
    const uint16_t fl = front_left_sensor_.readSpeed();
    const uint16_t fr = front_right_sensor_.readSpeed();
    const uint16_t rl = rear_left_sensor_.readSpeed();
    const uint16_t rr = rear_right_sensor_.readSpeed();

    vcansim_abs_status_t msg{};
    msg.wheel_fl = vcansim_abs_status_wheel_fl_encode(static_cast<float>(fl) * 0.1f);
    msg.wheel_fr = vcansim_abs_status_wheel_fr_encode(static_cast<float>(fr) * 0.1f);
    msg.wheel_rl = vcansim_abs_status_wheel_rl_encode(static_cast<float>(rl) * 0.1f);
    msg.wheel_rr = vcansim_abs_status_wheel_rr_encode(static_cast<float>(rr) * 0.1f);

    CanFrame frame{};
    frame.id  = VCANSIM_ABS_STATUS_FRAME_ID;
    frame.dlc = VCANSIM_ABS_STATUS_LENGTH;
    vcansim_abs_status_pack(frame.data.data(), &msg, frame.data.size());

    if (!driver_.send(frame)) {
        // send failed. no retry or error propagation in this simulator
    }
}