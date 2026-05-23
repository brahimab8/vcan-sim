#pragma once

#include <ostream>

#include "ican_driver.h"
#include "monitor_core.h"

class MonitorSession {
public:
    MonitorSession(ICanDriver& driver, MonitorCore& core, std::ostream& output);
    
    // Attempt to receive and process a single CAN frame. Returns true on success, false on failure.
    bool pumpOnce();
    
    // Runs the monitoring session, continuously receiving and processing CAN frames until stopped.
    void run();
    
    // Requests the monitoring session to stop.
    void stop();

private:
    ICanDriver& driver_;
    MonitorCore& core_;
    std::ostream& output_;
    bool running_{true};
};