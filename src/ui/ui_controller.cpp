#include "ui_controller.h"
#include "vcansim.h"

UiController::UiController(ICanDriver& driver)
    : client_(std::make_unique<MotorControlClient>(driver)) {}

std::optional<float> UiController::sendTargetRpm(float target_rpm)
{
    if (client_->sendTargetRpm(target_rpm))
        return target_rpm;
    return std::nullopt;
}

std::vector<EcuDescriptor> UiController::ecuDescriptors() const
{
    return {
        {
            "MotorECU", {
                { VCANSIM_MOTOR_STATUS_NAME,  VCANSIM_MOTOR_STATUS_RPM_NAME         },
                { VCANSIM_MOTOR_STATUS_NAME,  VCANSIM_MOTOR_STATUS_TEMPERATURE_NAME },
                { VCANSIM_MOTOR_CONTROL_NAME, VCANSIM_MOTOR_CONTROL_TARGET_RPM_NAME },
            }
        },
        {
            "ABSECU", {
                { VCANSIM_ABS_STATUS_NAME, VCANSIM_ABS_STATUS_WHEEL_FL_NAME },
                { VCANSIM_ABS_STATUS_NAME, VCANSIM_ABS_STATUS_WHEEL_FR_NAME },
                { VCANSIM_ABS_STATUS_NAME, VCANSIM_ABS_STATUS_WHEEL_RL_NAME },
                { VCANSIM_ABS_STATUS_NAME, VCANSIM_ABS_STATUS_WHEEL_RR_NAME },
            }
        },
    };
}

std::vector<ControlDescriptor> UiController::controlDescriptors() const
{
    const RpmRange r = client_->targetRpmRange();
    return {{
        VCANSIM_MOTOR_CONTROL_TARGET_RPM_NAME,  // label
        VCANSIM_MOTOR_CONTROL_NAME,             // message_name (for card update)
        VCANSIM_MOTOR_CONTROL_TARGET_RPM_NAME,  // signal_name (for card update)
        r.min, r.max, r.default_value
    }};
}