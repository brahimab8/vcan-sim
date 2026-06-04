#include "motor_control_client.h"

#include "vcansim.h"

MotorControlClient::MotorControlClient(ICanDriver& driver)
    : driver_(driver) 
    {   
    }

bool MotorControlClient::sendTargetRpm(float target_rpm)
{
    if (!vcansim_motor_control_target_rpm_is_in_phys_range(target_rpm)) {
        return false;
   }

    vcansim_motor_control_t msg{};
    msg.target_rpm = vcansim_motor_control_target_rpm_encode(target_rpm);

    CanFrame frame{};
    frame.id = VCANSIM_MOTOR_CONTROL_FRAME_ID;
    frame.dlc = VCANSIM_MOTOR_CONTROL_LENGTH;

    if (vcansim_motor_control_pack(frame.data.data(), &msg, frame.data.size()) < 0) {
        return false;
    }

    return driver_.send(frame);
}

RpmRange MotorControlClient::targetRpmRange() const
{
    return {
        MOTOR_CONTROL_TARGET_RPM_PHYS_MIN,
        MOTOR_CONTROL_TARGET_RPM_PHYS_MAX,
        MOTOR_CONTROL_TARGET_RPM_PHYS_DEFAULT
    };
}