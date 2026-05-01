#pragma once

#include "itimer.h"

// Linux implementation of ITimer using std::this_thread::sleep_for
class LinuxTimer final : public ITimer {
public:
    void sleepMs(uint32_t ms) override;
};