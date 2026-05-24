#include <gtest/gtest.h>

#include "mock_can_driver.h"
#include "mock_motor_controller.h"
#include "mock_sensor.h"
#include "mock_timer.h"
#include "vcansim.h"

#include "motor_ecu.h"

// ---------------------------------------------------------------------------
// Helper: construct a MotorEcu with fresh mocks
// ---------------------------------------------------------------------------
struct MotorEcuFixture {
    MockCanDriver driver;
    MockTimer     timer;
    MockRpmSensor       rpm_sensor{{800, 1200}};
    MockTempSensor      temp_sensor{{20, 45}};
    MockMotorController motor_controller;
    MotorEcu            ecu{driver, timer, rpm_sensor, temp_sensor, motor_controller};
};

// ---------------------------------------------------------------------------
// Frame structure
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

    EXPECT_EQ(f.driver.sentFrames()[0].id, VCANSIM_MOTOR_STATUS_FRAME_ID);
}

TEST(MotorEcuTick, FrameHasCorrectDlc)
{
    MotorEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.driver.sentFrames()[0].dlc, VCANSIM_MOTOR_STATUS_LENGTH);
}

// ---------------------------------------------------------------------------
// Signal values
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Timer isolation
// ---------------------------------------------------------------------------

TEST(MotorEcuTick, DoesNotCallTimer)
{
    MotorEcuFixture f;
    f.ecu.tick();

    EXPECT_EQ(f.timer.callCount(), 0U);
}

// ---------------------------------------------------------------------------
// MotorControl command handling
// ---------------------------------------------------------------------------

TEST(MotorEcuTick, IgnoresIncomingFrameWithWrongId)
{
    MotorEcuFixture f;

    CanFrame wrong{};
    wrong.id  = 0x999;
    wrong.dlc = 2;
    f.driver.enqueueReceiveFrame(wrong);

    f.ecu.tick();

    EXPECT_EQ(f.motor_controller.callCount(), 0U);
}

TEST(MotorEcuTick, ForwardsValidCommandToMotorController)
{
    MotorEcuFixture f;

    vcansim_motor_control_t cmd{};
    cmd.target_rpm = vcansim_motor_control_target_rpm_encode(3000.0f);

    CanFrame frame{};
    frame.id  = VCANSIM_MOTOR_CONTROL_FRAME_ID;
    frame.dlc = VCANSIM_MOTOR_CONTROL_LENGTH;
    vcansim_motor_control_pack(frame.data.data(), &cmd, frame.data.size());

    f.driver.enqueueReceiveFrame(frame);

    f.ecu.tick();

    ASSERT_EQ(f.motor_controller.callCount(), 1U);
    EXPECT_EQ(f.motor_controller.calls()[0], 3000U);
}