#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "base_ecu.h"
#include "itimer.h"

// EcuRunLimiterTimer: deterministic timer used by integration tests.
class EcuRunLimiterTimer final : public ITimer {
public:
    explicit EcuRunLimiterTimer(std::size_t stop_after)
        : stop_after_(stop_after)
    {
    }

    void attachEcu(BaseEcu& ecu)
    {
        target_ = &ecu;
    }

    void sleepMs(uint32_t ms) override
    {
        sleep_calls_.push_back(ms);

        if (target_ != nullptr && sleep_calls_.size() >= stop_after_) {
            target_->stop();
        }
    }

    [[nodiscard]] std::size_t callCount() const
    {
        return sleep_calls_.size();
    }

    [[nodiscard]] const std::vector<uint32_t>& sleepCalls() const
    {
        return sleep_calls_;
    }

private:
    std::size_t stop_after_;
    BaseEcu* target_{nullptr};
    std::vector<uint32_t> sleep_calls_;
};
