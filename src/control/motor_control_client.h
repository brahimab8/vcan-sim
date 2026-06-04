#pragma once

#include "ican_driver.h"

// Physical range of TargetRPM as defined in the DBC [0|8000].
// Lives here because MotorControlClient is the only layer that owns MotorControl signal semantics.
static constexpr float MOTOR_CONTROL_TARGET_RPM_PHYS_MIN     =    0.0f;
static constexpr float MOTOR_CONTROL_TARGET_RPM_PHYS_MAX     = 8000.0f;
static constexpr float MOTOR_CONTROL_TARGET_RPM_PHYS_DEFAULT =  800.0f;

struct RpmRange {
    float min;
    float max;
    float default_value;
};

class MotorControlClient final {
public:
    explicit MotorControlClient(ICanDriver& driver);

    bool sendTargetRpm(float target_rpm);
    RpmRange targetRpmRange() const;

private:
    ICanDriver& driver_;
};
