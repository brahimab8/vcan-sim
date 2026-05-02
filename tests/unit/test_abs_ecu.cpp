#include <gtest/gtest.h>

#include "abs_ecu.h"
#include "mock_can_driver.h"
#include "mock_sensor.h"
#include "mock_timer.h"
#include "signal_encoder.h"

// ---------------------------------------------------------------------------
// Helper: construct an AbsEcu with fresh mocks
// ---------------------------------------------------------------------------
struct AbsEcuFixture {
    MockCanDriver driver;
    MockTimer     timer;
    MockSensor<uint16_t> front_left_sensor{{0, 100}};
    MockSensor<uint16_t> front_right_sensor{{0, 102}};
    MockSensor<uint16_t> rear_left_sensor{{0, 98}};
    MockSensor<uint16_t> rear_right_sensor{{0, 99}};
    AbsEcu        ecu{driver, timer, front_left_sensor, front_right_sensor, rear_left_sensor, rear_right_sensor};
};

// ---------------------------------------------------------------------------
// Scaling helper: pure AbsEcu logic, no SignalEncoder involved
// ---------------------------------------------------------------------------

TEST(AbsEcuScaling, SpeedToRaw_Zero)
{
    EXPECT_EQ(AbsEcu::speedToRaw(0), 0U);
}

TEST(AbsEcuScaling, SpeedToRaw_TypicalValue)
{
    EXPECT_EQ(AbsEcu::speedToRaw(1000), 1000U);  // 100.0 km/h => 1000 deci-km/h
}

TEST(AbsEcuScaling, SpeedToRaw_MaxSpecValue)
{
    EXPECT_EQ(AbsEcu::speedToRaw(1200), 1200U);  // max profile value
}

TEST(AbsEcuScaling, SpeedToRaw_ClampsAboveSpecMax)
{
    EXPECT_EQ(AbsEcu::speedToRaw(3500), 3000U);  // clamp above 300.0 km/h
}

// ---------------------------------------------------------------------------
// Frame structure: verifies tick() builds a correctly structured frame
// ---------------------------------------------------------------------------

TEST(AbsEcuTick, SendsExactlyOneFrame)
{
    AbsEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.driver.sentFrameCount(), 1U);
}

TEST(AbsEcuTick, FrameHasCorrectCanId)
{
    AbsEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.driver.sentFrames()[0].id, 0x200U);
}

TEST(AbsEcuTick, FrameHasCorrectDlc)
{
    AbsEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.driver.sentFrames()[0].dlc, 8U);
}

// ---------------------------------------------------------------------------
// Signal values: verifies tick() encodes all four injected wheel speeds correctly
// SignalEncoder is used here as a trusted decode primitive (independently tested)
// ---------------------------------------------------------------------------

TEST(AbsEcuTick, FirstTickEncodesAllFourWheels)
{
    AbsEcuFixture f;
    f.ecu.tick();

    const CanFrame& frame = f.driver.sentFrames()[0];
    uint16_t fl = 0, fr = 0, rl = 0, rr = 0;

    SignalEncoder::decodeUint16LE(frame, 0, fl);
    SignalEncoder::decodeUint16LE(frame, 2, fr);
    SignalEncoder::decodeUint16LE(frame, 4, rl);
    SignalEncoder::decodeUint16LE(frame, 6, rr);

    // First injected sensor values
    EXPECT_EQ(fl, AbsEcu::speedToRaw(0));
    EXPECT_EQ(fr, AbsEcu::speedToRaw(0));
    EXPECT_EQ(rl, AbsEcu::speedToRaw(0));
    EXPECT_EQ(rr, AbsEcu::speedToRaw(0));
}

TEST(AbsEcuTick, SecondTickEncodesSecondProfileValues)
{
    AbsEcuFixture f;
    f.ecu.tick();  // index 0
    f.ecu.tick();  // index 1: FL=100, FR=102, RL=98, RR=99 (deci-km/h)

    const CanFrame& frame = f.driver.sentFrames()[1];
    uint16_t fl = 0, fr = 0, rl = 0, rr = 0;

    SignalEncoder::decodeUint16LE(frame, 0, fl);
    SignalEncoder::decodeUint16LE(frame, 2, fr);
    SignalEncoder::decodeUint16LE(frame, 4, rl);
    SignalEncoder::decodeUint16LE(frame, 6, rr);

    EXPECT_EQ(fl, AbsEcu::speedToRaw(100));
    EXPECT_EQ(fr, AbsEcu::speedToRaw(102));
    EXPECT_EQ(rl, AbsEcu::speedToRaw(98));
    EXPECT_EQ(rr, AbsEcu::speedToRaw(99));
}

// ---------------------------------------------------------------------------
// Sensor input progression: verifies the ECU reads updated values on later ticks
// ---------------------------------------------------------------------------

TEST(AbsEcuTick, ReadsUpdatedWheelSensorValuesOnEachTick)
{
    AbsEcuFixture f;
    f.ecu.tick();  // first injected sensor values
    f.ecu.tick();  // second injected sensor values

    uint16_t fl0 = 0, fl1 = 0;
    SignalEncoder::decodeUint16LE(f.driver.sentFrames()[0], 0, fl0);
    SignalEncoder::decodeUint16LE(f.driver.sentFrames()[1], 0, fl1);

    EXPECT_EQ(fl0, AbsEcu::speedToRaw(0));
    EXPECT_EQ(fl1, AbsEcu::speedToRaw(100));
}

// ---------------------------------------------------------------------------
// Timer isolation: tick() must not call the timer
// ---------------------------------------------------------------------------

TEST(AbsEcuTick, DoesNotCallTimer)
{
    AbsEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.timer.callCount(), 0U);
}