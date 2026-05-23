#include <gtest/gtest.h>

#include <cstring>
#include <filesystem>
#include <fstream>

#include "monitor_core.h"
#include "vcansim.h"

TEST(MonitorCoreTest, WritesDecodedFrameToCsv)
{
    namespace fs = std::filesystem;
    fs::path tmp = "data/test_monitor_core";
    fs::remove_all(tmp);

    MonitorCore core(std::nullopt, tmp.string());

    struct vcansim_motor_status_t src{};
    src.rpm = vcansim_motor_status_rpm_encode(1200.0f);
    src.temperature = vcansim_motor_status_temperature_encode(75.0f);

    uint8_t buf[8] = {};
    int packed = vcansim_motor_status_pack(buf, &src, sizeof(buf));
    ASSERT_EQ(packed, VCANSIM_MOTOR_STATUS_LENGTH);

    CanFrame frame{};
    frame.id = VCANSIM_MOTOR_STATUS_FRAME_ID;
    frame.dlc = packed;
    std::memcpy(frame.data.data(), buf, packed);

    std::string summary = core.processFrame(frame);
    EXPECT_NE(summary.find("msg=MotorStatus"), std::string::npos);
    EXPECT_NE(summary.find("id=0x"), std::string::npos);

    fs::path csv = tmp / "MotorStatus.csv";
    ASSERT_TRUE(fs::exists(csv));

    std::ifstream in(csv);
    std::string line;
    std::getline(in, line);
    EXPECT_EQ(line, "timestamp,frame_id,RPM,Temperature");
    std::getline(in, line);
    EXPECT_NE(line.find(",0x"), std::string::npos);
    EXPECT_NE(line.find(",1200"), std::string::npos);

    fs::remove_all(tmp);
}

TEST(MonitorCoreTest, FormatsUnknownFrameWithoutCsvLogging)
{
    MonitorCore core(std::nullopt, std::nullopt);

    CanFrame frame{};
    frame.id = 0x123;
    frame.dlc = 2;
    frame.data[0] = 0xAB;
    frame.data[1] = 0xCD;

    std::string summary = core.processFrame(frame);
    EXPECT_NE(summary.find("msg=MSG_0x123"), std::string::npos);
    EXPECT_NE(summary.find("id=0x123"), std::string::npos);
}