#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "abs_ecu.h"
#include "base_ecu.h"
#include "vcansim.h"

#include "mock_can_driver.h"
#include "mock_sensor.h"
#include "stop_timer.h"

// Integration tests for AbsEcu
// These tests exercise AbsEcu together with test doubles (MockCanDriver,
// MockSensor, and a deterministic timer).
TEST(AbsEcuIntegration, EncodingOnTickSequence)
{
    MockCanDriver         driver;
    EcuRunLimiterTimer timer{99};
    MockWheelSensor       fl{{0, 100, 200}};
    MockWheelSensor       fr{{0, 102, 202}};
    MockWheelSensor       rl{{0, 98, 198}};
    MockWheelSensor       rr{{0, 99, 199}};

    AbsEcu ecu{driver, timer, fl, fr, rl, rr};

    ecu.tick();
    ecu.tick();
    ecu.tick();

    // We expect three frames because we called tick() three times.
    ASSERT_EQ(driver.sentFrameCount(), 3U);

    vcansim_abs_status_t msg0{}, msg2{};
    vcansim_abs_status_unpack(&msg0, driver.sentFrames()[0].data.data(), driver.sentFrames()[0].dlc);
    vcansim_abs_status_unpack(&msg2, driver.sentFrames()[2].data.data(), driver.sentFrames()[2].dlc);

    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_fl_decode(msg0.wheel_fl), 0.0f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_fr_decode(msg0.wheel_fr), 0.0f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_rl_decode(msg0.wheel_rl), 0.0f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_rr_decode(msg0.wheel_rr), 0.0f);

    // sensor provides deci-km/h, ECU converts to km/h before encoding
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_fl_decode(msg2.wheel_fl), 20.0f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_fr_decode(msg2.wheel_fr), 20.2f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_rl_decode(msg2.wheel_rl), 19.8f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_rr_decode(msg2.wheel_rr), 19.9f);
}

TEST(AbsEcuIntegration, RunStopsAfterConfiguredCycles)
{
    MockCanDriver         driver;
    EcuRunLimiterTimer timer{4};
    MockWheelSensor       fl{{0, 100, 200, 300}};
    MockWheelSensor       fr{{0, 102, 202, 302}};
    MockWheelSensor       rl{{0, 98, 198, 298}};
    MockWheelSensor       rr{{0, 99, 199, 299}};

    // This test verifies the ECU run loop timing and stop behavior. The
    // EcuRunLimiterTimer (deterministic test timer) will call stop() on the
    // ECU after N sleep cycles, so we can run the loop and then assert the
    // number and interval of sleep calls as well as the number of sent frames.
    AbsEcu ecu{driver, timer, fl, fr, rl, rr};
    timer.attachEcu(ecu);

    // Run the loop until the timer signals a stop.
    ecu.run();

    // Timer should have recorded four sleep calls of 20 ms each (the ABS CYCLE_MS)
    EXPECT_EQ(timer.callCount(), 4U);
    ASSERT_EQ(timer.sleepCalls().size(), 4U);
    EXPECT_EQ(timer.sleepCalls()[0], 20U);
    EXPECT_EQ(timer.sleepCalls()[1], 20U);
    EXPECT_EQ(timer.sleepCalls()[2], 20U);
    EXPECT_EQ(timer.sleepCalls()[3], 20U);

    // One frame per loop iteration expected
    EXPECT_EQ(driver.sentFrameCount(), 4U);
}
