#pragma once

#include "ican_driver.h"

class MotorControlClient final {
public:
    explicit MotorControlClient(ICanDriver& driver);

    bool sendTargetRpm(float target_rpm);

private:
    ICanDriver& driver_;
};
