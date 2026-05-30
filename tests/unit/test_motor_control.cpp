#include <gtest/gtest.h>

#include <cstring>

#include "mock_can_driver.h"
#include "motor_control_client.h"
#include "vcansim.h"

TEST(MotorControlClientTest, SendsEncodedMotorControlFrame)
{
    MockCanDriver driver;
    MotorControlClient client(driver);

    ASSERT_TRUE(client.sendTargetRpm(3000.0f));
    ASSERT_EQ(driver.sentFrameCount(), 1u);

    const CanFrame& frame = driver.sentFrames().front();
    EXPECT_EQ(frame.id, VCANSIM_MOTOR_CONTROL_FRAME_ID);
    EXPECT_EQ(frame.dlc, VCANSIM_MOTOR_CONTROL_LENGTH);

    vcansim_motor_control_t msg{};
    ASSERT_EQ(vcansim_motor_control_unpack(&msg, frame.data.data(), frame.dlc), 0);
    EXPECT_FLOAT_EQ(vcansim_motor_control_target_rpm_decode(msg.target_rpm), 3000.0f);
}

TEST(MotorControlClientTest, RejectsOutOfRangeTargetRpm)
{
    MockCanDriver driver;
    MotorControlClient client(driver);

    EXPECT_FALSE(client.sendTargetRpm(-1.0f));
    EXPECT_EQ(driver.sentFrameCount(), 0u);
}
