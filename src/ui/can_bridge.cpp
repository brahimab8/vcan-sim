#include "can_bridge.h"

CanBridge::CanBridge(ICanDriver& driver, MonitorCore& core)
    : driver_(driver), core_(core)
{
}

bool CanBridge::runOnce()
{
    CanFrame frame{};
    if (!driver_.receive(frame)) {
        return false;
    }

    auto decoded = core_.decode(frame);
    if (!decoded.name.empty()) {
        if (on_message_) on_message_(decoded);
    }

    core_.logFrame(frame, decoded);
    return true;
}

void CanBridge::setOnMessage(std::function<void(const DecodedMessage&)> cb)
{
    on_message_ = std::move(cb);
}
