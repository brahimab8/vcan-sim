#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "abs_ecu.h"
#include "base_ecu.h"
#include "signal_encoder.h"

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
    MockSensor<uint16_t>  fl{{0, 100, 200}};
    MockSensor<uint16_t>  fr{{0, 102, 202}};
    MockSensor<uint16_t>  rl{{0, 98, 198}};
    MockSensor<uint16_t>  rr{{0, 99, 199}};

    AbsEcu ecu{driver, timer, fl, fr, rl, rr};

    ecu.tick();
    ecu.tick();
    ecu.tick();

    // We expect three frames because we called tick() three times.
    ASSERT_EQ(driver.sentFrameCount(), 3U);

    uint16_t fl0 = 0, fr0 = 0, rl0 = 0, rr0 = 0;
    uint16_t fl2 = 0, fr2 = 0, rl2 = 0, rr2 = 0;

    // Decode the first captured frame and verify wheel values are placed at
    // the expected byte offsets (0,2,4,6) as uint16 LE values.
    SignalEncoder::decodeUint16LE(driver.sentFrames()[0], 0, fl0);
    SignalEncoder::decodeUint16LE(driver.sentFrames()[0], 2, fr0);
    SignalEncoder::decodeUint16LE(driver.sentFrames()[0], 4, rl0);
    SignalEncoder::decodeUint16LE(driver.sentFrames()[0], 6, rr0);

    // Decode the third captured frame (index 2) to verify later profile values
    // were encoded correctly on subsequent ticks.
    SignalEncoder::decodeUint16LE(driver.sentFrames()[2], 0, fl2);
    SignalEncoder::decodeUint16LE(driver.sentFrames()[2], 2, fr2);
    SignalEncoder::decodeUint16LE(driver.sentFrames()[2], 4, rl2);
    SignalEncoder::decodeUint16LE(driver.sentFrames()[2], 6, rr2);

    EXPECT_EQ(fl0, AbsEcu::speedToRaw(0));
    EXPECT_EQ(fr0, AbsEcu::speedToRaw(0));
    EXPECT_EQ(rl0, AbsEcu::speedToRaw(0));
    EXPECT_EQ(rr0, AbsEcu::speedToRaw(0));

    EXPECT_EQ(fl2, AbsEcu::speedToRaw(200));
    EXPECT_EQ(fr2, AbsEcu::speedToRaw(202));
    EXPECT_EQ(rl2, AbsEcu::speedToRaw(198));
    EXPECT_EQ(rr2, AbsEcu::speedToRaw(199));
}

TEST(AbsEcuIntegration, RunStopsAfterConfiguredCycles)
{
    MockCanDriver         driver;
    EcuRunLimiterTimer timer{4};
    MockSensor<uint16_t>  fl{{0, 100, 200, 300}};
    MockSensor<uint16_t>  fr{{0, 102, 202, 302}};
    MockSensor<uint16_t>  rl{{0, 98, 198, 298}};
    MockSensor<uint16_t>  rr{{0, 99, 199, 299}};

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
