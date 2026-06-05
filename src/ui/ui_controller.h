#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ican_driver.h"
#include "motor_control_client.h"

struct SignalDescriptor {
    std::string message_name;
    std::string signal_name;
};

struct EcuDescriptor {
    std::string                   name;
    std::vector<SignalDescriptor> signal_list;
};

struct ControlDescriptor {
    std::string label;
    std::string message_name;  // so MainWindow knows which card to update
    std::string signal_name;
    float       min;
    float       max;
    float       default_value;
};

class UiController {
public:
    explicit UiController(ICanDriver& driver);

    std::vector<EcuDescriptor>     ecuDescriptors()     const;
    std::vector<ControlDescriptor> controlDescriptors() const;

    // Returns the sent value on success, nullopt on failure.
    std::optional<float> sendTargetRpm(float target_rpm);

private:
    std::unique_ptr<MotorControlClient> client_;
};