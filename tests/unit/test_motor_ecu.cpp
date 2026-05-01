#include <gtest/gtest.h>

#include "mock_can_driver.h"
#include "mock_timer.h"
#include "motor_ecu.h"
#include "signal_encoder.h"

// ---------------------------------------------------------------------------
// Helper: construct a MotorEcu with fresh mocks
// ---------------------------------------------------------------------------
struct MotorEcuFixture {
    MockCanDriver driver;
    MockTimer     timer;
    MotorEcu      ecu{driver, timer};
};

// ---------------------------------------------------------------------------
// Scaling helpers: verifies the static scaling functions produce expected raw values
// ---------------------------------------------------------------------------

TEST(MotorEcuScaling, RpmToRaw_ZeroRpm)
{
    EXPECT_EQ(MotorEcu::rpmToRaw(0), 0U);
}

TEST(MotorEcuScaling, RpmToRaw_TypicalValue)
{
    EXPECT_EQ(MotorEcu::rpmToRaw(800), 1600U);
}

TEST(MotorEcuScaling, RpmToRaw_MaxValue)
{
    EXPECT_EQ(MotorEcu::rpmToRaw(8000), 16000U);
}

TEST(MotorEcuScaling, TempToRaw_MinValue)
{
    EXPECT_EQ(MotorEcu::tempToRaw(-40), 0U);   // offset -40 -> raw = 0
}

TEST(MotorEcuScaling, TempToRaw_TypicalValue)
{
    EXPECT_EQ(MotorEcu::tempToRaw(20), 60U);   // 20 + 40 = 60
}

TEST(MotorEcuScaling, TempToRaw_MaxValue)
{
    EXPECT_EQ(MotorEcu::tempToRaw(150), 190U); // 150 + 40 = 190
}

// ---------------------------------------------------------------------------
// Frame structure: verifies tick() builds a correctly structured frame
// ---------------------------------------------------------------------------

TEST(MotorEcuTick, SendsExactlyOneFrame)
{
    MotorEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.driver.sentFrameCount(), 1U);
}

TEST(MotorEcuTick, FrameHasCorrectCanId)
{
    MotorEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.driver.sentFrames()[0].id, 0x100U);
}

TEST(MotorEcuTick, FrameHasCorrectDlc)
{
    MotorEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.driver.sentFrames()[0].dlc, 3U);
}

// ---------------------------------------------------------------------------
// Signal values: verifies tick() encodes profile values correctly
// SignalEncoder is used here as a trusted decode primitive (independently tested)
// ---------------------------------------------------------------------------

TEST(MotorEcuTick, FirstTickEncodesFirstRpmProfile)
{
    MotorEcuFixture f;
    f.ecu.tick();

    uint16_t raw = 0;
    SignalEncoder::decodeUint16LE(f.driver.sentFrames()[0], 0, raw);
    EXPECT_EQ(raw, MotorEcu::rpmToRaw(800));  // first profile value
}

TEST(MotorEcuTick, FirstTickEncodesFirstTempProfile)
{
    MotorEcuFixture f;
    f.ecu.tick();

    uint8_t raw = 0;
    SignalEncoder::decodeUint8(f.driver.sentFrames()[0], 2, raw);
    EXPECT_EQ(raw, MotorEcu::tempToRaw(20));  // first profile value
}

// ---------------------------------------------------------------------------
// Profile cycling: verifies profile advances and wraps correctly
// ---------------------------------------------------------------------------

TEST(MotorEcuProfile, AdvancesOnEachTick)
{
    MotorEcuFixture f;
    f.ecu.tick();  // index 0: rpm=800
    f.ecu.tick();  // index 1: rpm=1200

    uint16_t raw0 = 0;
    uint16_t raw1 = 0;
    SignalEncoder::decodeUint16LE(f.driver.sentFrames()[0], 0, raw0);
    SignalEncoder::decodeUint16LE(f.driver.sentFrames()[1], 0, raw1);

    EXPECT_EQ(raw0, MotorEcu::rpmToRaw(800));
    EXPECT_EQ(raw1, MotorEcu::rpmToRaw(1200));
}

TEST(MotorEcuProfile, WrapsAroundAfterAllValues)
{
    MotorEcuFixture f;

    for (int i = 0; i < 8; ++i) {
        f.ecu.tick();
    }
    f.ecu.tick();  // 9th tick wraps to index 0

    uint16_t raw = 0;
    SignalEncoder::decodeUint16LE(f.driver.sentFrames()[8], 0, raw);
    EXPECT_EQ(raw, MotorEcu::rpmToRaw(800));  // back to first profile value
}

// ---------------------------------------------------------------------------
// Timer isolation: tick() must not call the timer
// ---------------------------------------------------------------------------

TEST(MotorEcuTick, DoesNotCallTimer)
{
    MotorEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.timer.callCount(), 0U);
}