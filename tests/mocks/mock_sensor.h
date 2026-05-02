#pragma once

#include <cstddef>
#include <initializer_list>
#include <vector>

#include "isensor.h"

template <typename T>
class MockSensor final : public ISensor<T> {
public:
    MockSensor(std::initializer_list<T> values)
        : values_(values)
    {
    }

    T read() noexcept override
    {
        if (values_.empty()) {
            return T{};
        }

        if (index_ < values_.size()) {
            return values_[index_++];
        }

        return values_.back();
    }

private:
    std::vector<T> values_;
    std::size_t    index_{0};
};