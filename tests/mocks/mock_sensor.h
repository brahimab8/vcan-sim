#pragma once

#include <cstdint>
#include <initializer_list>
#include <vector>

#include "isensors.h"

template<typename TInterface, typename TValue>
class MockSensor : public TInterface {
public:
    explicit MockSensor(std::initializer_list<TValue> values)
        : values_(values) {}

protected:
    TValue next() noexcept
    {
        const TValue value = values_[index_ % values_.size()];
        ++index_;
        return value;
    }

private:
    std::vector<TValue> values_;
    std::size_t         index_{0};
};

class MockRpmSensor final : public MockSensor<IRpmSensor, uint16_t> {
public:
    using MockSensor::MockSensor;
    uint16_t readRpm() noexcept override { return next(); }
};

class MockTempSensor final : public MockSensor<ITempSensor, int16_t> {
public:
    using MockSensor::MockSensor;
    int16_t readTemp() noexcept override { return next(); }
};

class MockWheelSensor final : public MockSensor<IWheelSensor, uint16_t> {
public:
    using MockSensor::MockSensor;
    uint16_t readSpeed() noexcept override { return next(); }
};