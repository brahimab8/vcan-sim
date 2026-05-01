#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "itimer.h"

// Fake timer for unit testing (never actually sleeps)
// Records sleep durations so tests can verify timing behavior if needed
class MockTimer final : public ITimer {
public:
    void sleepMs(uint32_t ms) override
    {
        sleep_calls_.push_back(ms);
    }

    [[nodiscard]] std::size_t callCount() const { return sleep_calls_.size(); }
    [[nodiscard]] const std::vector<uint32_t>& sleepCalls() const { return sleep_calls_; }

    void clear() { sleep_calls_.clear(); }

private:
    std::vector<uint32_t> sleep_calls_;
};