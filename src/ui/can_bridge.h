#pragma once

#include <functional>

#include "ican_driver.h"
#include "monitor_core.h"

class CanBridge {
public:
    CanBridge(ICanDriver& driver, MonitorCore& core);

    // Run a single receive/decode iteration. Returns true if a frame was
    // processed, false if no frame was available.
    bool runOnce();

    // Optional callback invoked for each decoded message (called from
    // the thread executing runOnce()). Use to forward messages to a GUI
    // thread via a safe mechanism (e.g. Qt signal/slot).
    void setOnMessage(std::function<void(const DecodedMessage&)> cb);

private:
    ICanDriver& driver_;
    MonitorCore& core_;
    std::function<void(const DecodedMessage&)> on_message_;
};
