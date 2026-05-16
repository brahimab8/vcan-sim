#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "base_ecu.h"
#include "motor_ecu.h"
#include "vcansim.h"

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

    vcansim_motor_status_t msg0{}, msg1{}, msg2{};
    vcansim_motor_status_unpack(&msg0, driver.sentFrames()[0].data.data(), driver.sentFrames()[0].dlc);
    vcansim_motor_status_unpack(&msg1, driver.sentFrames()[1].data.data(), driver.sentFrames()[1].dlc);
    vcansim_motor_status_unpack(&msg2, driver.sentFrames()[2].data.data(), driver.sentFrames()[2].dlc);

    EXPECT_FLOAT_EQ(vcansim_motor_status_rpm_decode(msg0.rpm),  800.0f);
    EXPECT_FLOAT_EQ(vcansim_motor_status_rpm_decode(msg1.rpm), 1200.0f);
    EXPECT_FLOAT_EQ(vcansim_motor_status_rpm_decode(msg2.rpm), 2000.0f);

    EXPECT_FLOAT_EQ(vcansim_motor_status_temperature_decode(msg0.temperature), 20.0f);
    EXPECT_FLOAT_EQ(vcansim_motor_status_temperature_decode(msg1.temperature), 45.0f);
    EXPECT_FLOAT_EQ(vcansim_motor_status_temperature_decode(msg2.temperature), 90.0f);
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
