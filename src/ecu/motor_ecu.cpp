#include "motor_ecu.h"

#include "signal_encoder.h"

MotorEcu::MotorEcu(ICanDriver& driver, ITimer& timer)
    : BaseEcu(driver, timer)
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

uint16_t MotorEcu::rpmToRaw(uint16_t rpm)
{
    return static_cast<uint16_t>(rpm * RPM_SCALE_FACTOR);
}

uint8_t MotorEcu::tempToRaw(int16_t temp)
{
    return static_cast<uint8_t>(temp + TEMP_OFFSET);
}


void MotorEcu::tick()
{
    const uint16_t rpm  = RPM_PROFILE[profile_index_];
    const int16_t  temp = TEMP_PROFILE[profile_index_];

    profile_index_ = (profile_index_ + 1) % RPM_PROFILE.size();

    CanFrame frame{};
    frame.id  = CAN_ID;
    frame.dlc = FRAME_DLC;

    // Convert RPM and temperature to raw values according to the DBC definitions 
    // And encode into the frame
    SignalEncoder::encodeUint16LE(frame, 0, rpmToRaw(rpm)); // RPM at offset 0-1
    SignalEncoder::encodeUint8(frame, 2, tempToRaw(temp));  // Temp at offset 2

    // Send the frame. If sending fails, skip this cycle and try again next time.
    if (!driver_.send(frame)) {
        // error handling deferred to future iterations.
    }
}