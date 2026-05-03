#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "base_ecu.h"
#include "signal_encoder.h"
#include "motor_ecu.h"

#include "mock_can_driver.h"
#include "mock_sensor.h"
#include "stop_timer.h"

// Integration tests for MotorEcu
// These tests exercise MotorEcu together with test doubles (MockCanDriver,
// MockSensor, and a deterministic timer). 

TEST(MotorEcuIntegration, EncodingOnTickSequence)
{
    MockCanDriver        driver;
    EcuRunLimiterTimer   timer{99};
    MockSensor<uint16_t> rpm_sensor{{800, 1200, 2000}};
    MockSensor<int16_t>  temp_sensor{{20, 45, 90}};

    MotorEcu ecu{driver, timer, rpm_sensor, temp_sensor};

    ecu.tick();
    ecu.tick();
    ecu.tick();

    // We expect three frames because we called tick() three times.
    ASSERT_EQ(driver.sentFrameCount(), 3U);

    uint16_t rpm0 = 0, rpm1 = 0, rpm2 = 0;
    uint8_t temp0 = 0, temp1 = 0, temp2 = 0;

    // Decode RPM (uint16 LE) from offset 0 of each captured frame and
    // temperature (uint8) from offset 2. These positions are defined by the
    // DBC and by the MotorEcu implementation.
    SignalEncoder::decodeUint16LE(driver.sentFrames()[0], 0, rpm0);
    SignalEncoder::decodeUint16LE(driver.sentFrames()[1], 0, rpm1);
    SignalEncoder::decodeUint16LE(driver.sentFrames()[2], 0, rpm2);

    SignalEncoder::decodeUint8(driver.sentFrames()[0], 2, temp0);
    SignalEncoder::decodeUint8(driver.sentFrames()[1], 2, temp1);
    SignalEncoder::decodeUint8(driver.sentFrames()[2], 2, temp2);

    EXPECT_EQ(rpm0, MotorEcu::rpmToRaw(800));
    EXPECT_EQ(rpm1, MotorEcu::rpmToRaw(1200));
    EXPECT_EQ(rpm2, MotorEcu::rpmToRaw(2000));

    EXPECT_EQ(temp0, MotorEcu::tempToRaw(20));
    EXPECT_EQ(temp1, MotorEcu::tempToRaw(45));
    EXPECT_EQ(temp2, MotorEcu::tempToRaw(90));
}

TEST(MotorEcuIntegration, RunStopsAfterConfiguredCycles)
{
    MockCanDriver         driver;
    EcuRunLimiterTimer timer{3};
    MockSensor<uint16_t>  rpm_sensor{{800, 1200, 2000}};
    MockSensor<int16_t>   temp_sensor{{20, 45, 90}};

    // This test verifies the run-loop timing and stop behavior. The
    // EcuRunLimiterTimer will stop the ECU after N sleep cycles, allowing us
    // to assert that `run()` calls sleep() with the configured Motor cycle
    // time (100 ms) and that a frame is sent each iteration.
    MotorEcu ecu{driver, timer, rpm_sensor, temp_sensor};
    timer.attachEcu(ecu);

    ecu.run();

    EXPECT_EQ(timer.callCount(), 3U);
    ASSERT_EQ(timer.sleepCalls().size(), 3U);
    EXPECT_EQ(timer.sleepCalls()[0], 100U);
    EXPECT_EQ(timer.sleepCalls()[1], 100U);
    EXPECT_EQ(timer.sleepCalls()[2], 100U);

    // One frame per loop iteration expected
    EXPECT_EQ(driver.sentFrameCount(), 3U);
}
