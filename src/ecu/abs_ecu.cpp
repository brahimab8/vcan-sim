#include "abs_ecu.h"

#include "signal_encoder.h"

AbsEcu::AbsEcu(
    ICanDriver& driver,
    ITimer& timer,
    WheelSensor& front_left_sensor,
    WheelSensor& front_right_sensor,
    WheelSensor& rear_left_sensor,
    WheelSensor& rear_right_sensor)
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

uint16_t AbsEcu::speedToRaw(uint16_t speed_deci_kmh)
{
    if (speed_deci_kmh > MAX_WHEEL_RAW) {
        return MAX_WHEEL_RAW;
    }
    return speed_deci_kmh;
}

void AbsEcu::tick()
{
    const uint16_t fl = front_left_sensor_.read();
    const uint16_t fr = front_right_sensor_.read();
    const uint16_t rl = rear_left_sensor_.read();
    const uint16_t rr = rear_right_sensor_.read();

    CanFrame frame{};
    frame.id  = CAN_ID;
    frame.dlc = FRAME_DLC;

    // Wheel sensors provide deci-km/h values directly as raw (scale 0.1).
    SignalEncoder::encodeUint16LE(frame, 0, speedToRaw(fl));
    SignalEncoder::encodeUint16LE(frame, 2, speedToRaw(fr));
    SignalEncoder::encodeUint16LE(frame, 4, speedToRaw(rl));
    SignalEncoder::encodeUint16LE(frame, 6, speedToRaw(rr));

    if (!driver_.send(frame)) {
        // send failed. no retry or error propagation in this simulator
    }
}