#include "monitor_session.h"

#include <chrono>
#include <thread>

MonitorSession::MonitorSession(ICanDriver& driver, MonitorCore& core, std::ostream& output)
    : driver_(driver), core_(core), output_(output)
{
}

bool MonitorSession::pumpOnce()
{
    CanFrame frame{};
    if (!driver_.receive(frame)) {
        return false;
    }

    output_ << core_.processFrame(frame) << '\n';
    return true;
}

void MonitorSession::run()
{
    while (running_) {
        if (!pumpOnce()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void MonitorSession::stop()
{
    running_ = false;
}