#include "decoder.h"

#include <cstring>

#include "vcansim.h"

DecodedMessage decodeFrameWithDbc(const CanFrame& frame) noexcept
{
    DecodedMessage out;
    switch (frame.id) {
        case VCANSIM_MOTOR_STATUS_FRAME_ID: {
            struct vcansim_motor_status_t msg{};
            if (frame.dlc < VCANSIM_MOTOR_STATUS_LENGTH) return out;
            if (vcansim_motor_status_unpack(&msg, frame.data.data(), frame.dlc) != 0) return out;
            out.name = VCANSIM_MOTOR_STATUS_NAME;
            out.columns = {VCANSIM_MOTOR_STATUS_RPM_NAME, VCANSIM_MOTOR_STATUS_TEMPERATURE_NAME};
            out.values = { std::to_string(vcansim_motor_status_rpm_decode(msg.rpm)),
                           std::to_string(vcansim_motor_status_temperature_decode(msg.temperature)) };
            return out;
        }

        case VCANSIM_ABS_STATUS_FRAME_ID: {
            struct vcansim_abs_status_t msg{};
            if (frame.dlc < VCANSIM_ABS_STATUS_LENGTH) return out;
            if (vcansim_abs_status_unpack(&msg, frame.data.data(), frame.dlc) != 0) return out;
            out.name = VCANSIM_ABS_STATUS_NAME;
            out.columns = {VCANSIM_ABS_STATUS_WHEEL_FL_NAME, VCANSIM_ABS_STATUS_WHEEL_FR_NAME,
                           VCANSIM_ABS_STATUS_WHEEL_RL_NAME, VCANSIM_ABS_STATUS_WHEEL_RR_NAME};
            out.values = {
                std::to_string(vcansim_abs_status_wheel_fl_decode(msg.wheel_fl)),
                std::to_string(vcansim_abs_status_wheel_fr_decode(msg.wheel_fr)),
                std::to_string(vcansim_abs_status_wheel_rl_decode(msg.wheel_rl)),
                std::to_string(vcansim_abs_status_wheel_rr_decode(msg.wheel_rr)) };
            return out;
        }

        case VCANSIM_MOTOR_CONTROL_FRAME_ID: {
            struct vcansim_motor_control_t msg{};
            if (frame.dlc < VCANSIM_MOTOR_CONTROL_LENGTH) return out;
            if (vcansim_motor_control_unpack(&msg, frame.data.data(), frame.dlc) != 0) return out;
            out.name = VCANSIM_MOTOR_CONTROL_NAME;
            out.columns = {VCANSIM_MOTOR_CONTROL_TARGET_RPM_NAME};
            out.values = { std::to_string(vcansim_motor_control_target_rpm_decode(msg.target_rpm)) };
            return out;
        }

        default:
            return out;
    }
}
