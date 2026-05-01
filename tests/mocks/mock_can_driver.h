#pragma once

#include <cstddef>
#include <deque>
#include <vector>

#include "ican_driver.h"

// Fake CAN driver for unit testing. No OS or hardware dependency
// Implements ICanDriver by recording sent frames and optionally queuing frames to receive
class MockCanDriver final : public ICanDriver {
public:
    bool send(const CanFrame& frame) override
    {
        sent_frames_.push_back(frame);
        return true;
    }

    bool receive(CanFrame& frame) override
    {
        if (receive_queue_.empty()) {
            return false;
        }

        frame = receive_queue_.front();
        receive_queue_.pop_front();
        return true;
    }

    void enqueueReceiveFrame(const CanFrame& frame)
    {
        receive_queue_.push_back(frame);
    }

    void clear()
    {
        sent_frames_.clear();
        receive_queue_.clear();
    }

    [[nodiscard]] const std::vector<CanFrame>& sentFrames() const
    {
        return sent_frames_;
    }

    [[nodiscard]] std::size_t sentFrameCount() const
    {
        return sent_frames_.size();
    }

    [[nodiscard]] bool hasReceiveFrame() const
    {
        return !receive_queue_.empty();
    }

private:
    std::vector<CanFrame> sent_frames_;
    std::deque<CanFrame> receive_queue_;
};