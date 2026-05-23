#include <gtest/gtest.h>

#include <cstring>

#include "decoder.h"
#include "vcansim.h"

TEST(DecoderTest, DecodesMotorStatus)
{
    struct vcansim_motor_status_t src{};
    src.rpm = vcansim_motor_status_rpm_encode(1200.0f); // physical 1200 rpm -> encoded
    src.temperature = vcansim_motor_status_temperature_encode(75.0f);

    uint8_t buf[8] = {};
    int packed = vcansim_motor_status_pack(buf, &src, sizeof(buf));
    ASSERT_EQ(packed, VCANSIM_MOTOR_STATUS_LENGTH);

    CanFrame frame{};
    frame.id = VCANSIM_MOTOR_STATUS_FRAME_ID;
    frame.dlc = packed;
    std::memcpy(frame.data.data(), buf, packed);

    auto decoded = decodeFrameWithDbc(frame);
    EXPECT_EQ(decoded.name, std::string(VCANSIM_MOTOR_STATUS_NAME));
    ASSERT_EQ(decoded.columns.size(), 2u);
    ASSERT_EQ(decoded.values.size(), 2u);
}

TEST(DecoderTest, DecodesAbsStatus)
{
    struct vcansim_abs_status_t src{};
    src.wheel_fl = vcansim_abs_status_wheel_fl_encode(12.3f);
    src.wheel_fr = vcansim_abs_status_wheel_fr_encode(45.6f);
    src.wheel_rl = vcansim_abs_status_wheel_rl_encode(7.8f);
    src.wheel_rr = vcansim_abs_status_wheel_rr_encode(9.0f);

    uint8_t buf[8] = {};
    int packed = vcansim_abs_status_pack(buf, &src, sizeof(buf));
    ASSERT_EQ(packed, VCANSIM_ABS_STATUS_LENGTH);

    CanFrame frame{};
    frame.id = VCANSIM_ABS_STATUS_FRAME_ID;
    frame.dlc = packed;
    std::memcpy(frame.data.data(), buf, packed);

    auto decoded = decodeFrameWithDbc(frame);
    EXPECT_EQ(decoded.name, std::string(VCANSIM_ABS_STATUS_NAME));
    ASSERT_EQ(decoded.columns.size(), 4u);
    ASSERT_EQ(decoded.values.size(), 4u);
}
