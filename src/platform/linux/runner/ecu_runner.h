#pragma once

#include <csignal>
#include <iostream>
#include <string>

#include "base_ecu.h"

namespace vcan_sim::runner {

// Global pointer to the currently active ECU instance for signal handling.
inline BaseEcu* active_ecu = nullptr;

inline void handleSignal(int)
{
    if (active_ecu != nullptr) {
        active_ecu->stop();
    }
}

inline void installSignalHandlers(BaseEcu& ecu)
{
    active_ecu = &ecu;
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);
}

inline int runEcuProcess(const std::string& label, BaseEcu& ecu)
{
    std::cout << '[' << label << "] starting on vcan0 (Ctrl+C to stop)" << std::endl;
    ecu.run();
    std::cout << '[' << label << "] stopped" << std::endl;
    return 0;
}

} // namespace vcan_sim::runner