#include <gtest/gtest.h>

#include "mock_can_driver.h"
#include "mock_sensor.h"
#include "mock_timer.h"
#include "motor_ecu.h"
#include "vcansim.h" // Generated from vcansim.dbc by cantools

// ---------------------------------------------------------------------------
// Helper: construct a MotorEcu with fresh mocks
// ---------------------------------------------------------------------------
struct MotorEcuFixture {
    MockCanDriver driver;
    MockTimer     timer;
    MockSensor<uint16_t> rpm_sensor{{800, 1200}};
    MockSensor<int16_t>  temp_sensor{{20, 45}};
    MotorEcu      ecu{driver, timer, rpm_sensor, temp_sensor};
};

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

    EXPECT_EQ(f.driver.sentFrames()[0].id, VCANSIM_MOTOR_STATUS_FRAME_ID);
}

TEST(MotorEcuTick, FrameHasCorrectDlc)
{
    MotorEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.driver.sentFrames()[0].dlc, VCANSIM_MOTOR_STATUS_LENGTH);
}

TEST(MotorEcuTick, FirstTickEncodesFirstRpmProfile)
{
    MotorEcuFixture f;
    f.ecu.tick();

    vcansim_motor_status_t msg{};
    vcansim_motor_status_unpack(&msg, f.driver.sentFrames()[0].data.data(),
                                f.driver.sentFrames()[0].dlc);

    EXPECT_FLOAT_EQ(vcansim_motor_status_rpm_decode(msg.rpm), 800.0f);
}

TEST(MotorEcuTick, FirstTickEncodesFirstTempProfile)
{
    MotorEcuFixture f;
    f.ecu.tick();

    vcansim_motor_status_t msg{};
    vcansim_motor_status_unpack(&msg, f.driver.sentFrames()[0].data.data(),
                                f.driver.sentFrames()[0].dlc);

    EXPECT_FLOAT_EQ(vcansim_motor_status_temperature_decode(msg.temperature), 20.0f);
}

TEST(MotorEcuTick, ReadsUpdatedSensorValuesOnEachTick)
{
    MotorEcuFixture f;
    f.ecu.tick();
    f.ecu.tick();

    vcansim_motor_status_t msg0{}, msg1{};
    vcansim_motor_status_unpack(&msg0, f.driver.sentFrames()[0].data.data(),
                                f.driver.sentFrames()[0].dlc);
    vcansim_motor_status_unpack(&msg1, f.driver.sentFrames()[1].data.data(),
                                f.driver.sentFrames()[1].dlc);

    EXPECT_FLOAT_EQ(vcansim_motor_status_rpm_decode(msg0.rpm), 800.0f);
    EXPECT_FLOAT_EQ(vcansim_motor_status_rpm_decode(msg1.rpm), 1200.0f);
}

TEST(MotorEcuTick, DoesNotCallTimer)
{
    MotorEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.timer.callCount(), 0U);
}