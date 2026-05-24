#include <gtest/gtest.h>

#include "abs_ecu.h"
#include "mock_can_driver.h"
#include "mock_sensor.h"
#include "mock_timer.h"
#include "vcansim.h"

// ---------------------------------------------------------------------------
// Helper: construct an AbsEcu with fresh mocks
// ---------------------------------------------------------------------------
struct AbsEcuFixture {
    MockCanDriver driver;
    MockTimer     timer;
    MockWheelSensor front_left_sensor{{0, 100}};
    MockWheelSensor front_right_sensor{{0, 102}};
    MockWheelSensor rear_left_sensor{{0, 98}};
    MockWheelSensor rear_right_sensor{{0, 99}};
    AbsEcu        ecu{driver, timer, front_left_sensor, front_right_sensor, rear_left_sensor, rear_right_sensor};
};

// ---------------------------------------------------------------------------
// Frame structure
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
    EXPECT_EQ(f.driver.sentFrames()[0].id, VCANSIM_ABS_STATUS_FRAME_ID);
}

TEST(AbsEcuTick, FrameHasCorrectDlc)
{
    AbsEcuFixture f;
    f.ecu.tick();
    EXPECT_EQ(f.driver.sentFrames()[0].dlc, VCANSIM_ABS_STATUS_LENGTH);
}

// ---------------------------------------------------------------------------
// Signal values
// ---------------------------------------------------------------------------

TEST(AbsEcuTick, FirstTickEncodesAllFourWheels)
{
    AbsEcuFixture f;
    f.ecu.tick();

    vcansim_abs_status_t msg{};
    vcansim_abs_status_unpack(&msg, f.driver.sentFrames()[0].data.data(),
                              f.driver.sentFrames()[0].dlc);

    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_fl_decode(msg.wheel_fl), 0.0f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_fr_decode(msg.wheel_fr), 0.0f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_rl_decode(msg.wheel_rl), 0.0f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_rr_decode(msg.wheel_rr), 0.0f);
}

TEST(AbsEcuTick, SecondTickEncodesSecondProfileValues)
{
    AbsEcuFixture f;
    f.ecu.tick();
    f.ecu.tick();

    vcansim_abs_status_t msg{};
    vcansim_abs_status_unpack(&msg, f.driver.sentFrames()[1].data.data(),
                              f.driver.sentFrames()[1].dlc);

    // sensor provides deci-km/h, ECU converts to km/h before encoding
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_fl_decode(msg.wheel_fl), 10.0f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_fr_decode(msg.wheel_fr), 10.2f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_rl_decode(msg.wheel_rl), 9.8f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_rr_decode(msg.wheel_rr), 9.9f);
}

TEST(AbsEcuTick, ReadsUpdatedWheelSensorValuesOnEachTick)
{
    AbsEcuFixture f;
    f.ecu.tick();
    f.ecu.tick();

    vcansim_abs_status_t msg0{}, msg1{};
    vcansim_abs_status_unpack(&msg0, f.driver.sentFrames()[0].data.data(),
                              f.driver.sentFrames()[0].dlc);
    vcansim_abs_status_unpack(&msg1, f.driver.sentFrames()[1].data.data(),
                              f.driver.sentFrames()[1].dlc);

    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_fl_decode(msg0.wheel_fl), 0.0f);
    EXPECT_FLOAT_EQ(vcansim_abs_status_wheel_fl_decode(msg1.wheel_fl), 10.0f);
}

// ---------------------------------------------------------------------------
// Timer isolation
// ---------------------------------------------------------------------------

TEST(AbsEcuTick, DoesNotCallTimer)
{
    AbsEcuFixture f;
    f.ecu.tick();
    EXPECT_EQ(f.timer.callCount(), 0U);
}
