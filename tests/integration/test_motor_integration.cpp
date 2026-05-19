#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "base_ecu.h"
#include "motor_ecu.h"
#include "vcansim.h"

#include "mock_can_driver.h"
#include "mock_motor_controller.h"
#include "mock_sensor.h"
#include "stop_timer.h"

// Integration tests for MotorEcu
// These tests exercise MotorEcu together with test doubles (MockCanDriver,
// MockRpmSensor, MockTempSensor, MockMotorController, and a deterministic timer).

TEST(MotorEcuIntegration, EncodingOnTickSequence)
{
    MockCanDriver        driver;
    EcuRunLimiterTimer   timer{99};
    MockRpmSensor        rpm_sensor{{800, 1200, 2000}};
    MockTempSensor       temp_sensor{{20, 45, 90}};
    MockMotorController  motor_controller;

    MotorEcu ecu{driver, timer, rpm_sensor, temp_sensor, motor_controller};

    ecu.tick();
    ecu.tick();
    ecu.tick();

    ASSERT_EQ(driver.sentFrameCount(), 3U);

    vcansim_motor_status_t msg0{}, msg1{}, msg2{};
    vcansim_motor_status_unpack(&msg0, driver.sentFrames()[0].data.data(), driver.sentFrames()[0].dlc);
    vcansim_motor_status_unpack(&msg1, driver.sentFrames()[1].data.data(), driver.sentFrames()[1].dlc);
    vcansim_motor_status_unpack(&msg2, driver.sentFrames()[2].data.data(), driver.sentFrames()[2].dlc);

    EXPECT_FLOAT_EQ(vcansim_motor_status_rpm_decode(msg0.rpm),  800.0f);
    EXPECT_FLOAT_EQ(vcansim_motor_status_rpm_decode(msg1.rpm), 1200.0f);
    EXPECT_FLOAT_EQ(vcansim_motor_status_rpm_decode(msg2.rpm), 2000.0f);

    EXPECT_FLOAT_EQ(vcansim_motor_status_temperature_decode(msg0.temperature), 20.0f);
    EXPECT_FLOAT_EQ(vcansim_motor_status_temperature_decode(msg1.temperature), 45.0f);
    EXPECT_FLOAT_EQ(vcansim_motor_status_temperature_decode(msg2.temperature), 90.0f);
}

TEST(MotorEcuIntegration, RunStopsAfterConfiguredCycles)
{
    MockCanDriver       driver;
    EcuRunLimiterTimer  timer{3};
    MockRpmSensor       rpm_sensor{{800, 1200, 2000}};
    MockTempSensor      temp_sensor{{20, 45, 90}};
    MockMotorController motor_controller;

    // This test verifies the run-loop timing and stop behavior. The
    // EcuRunLimiterTimer will stop the ECU after N sleep cycles, allowing us
    // to assert that `run()` calls sleep() with the configured Motor cycle
    // time (100 ms) and that a frame is sent each iteration.
    MotorEcu ecu{driver, timer, rpm_sensor, temp_sensor, motor_controller};
    timer.attachEcu(ecu);

    ecu.run();

    EXPECT_EQ(timer.callCount(), 3U);
    ASSERT_EQ(timer.sleepCalls().size(), 3U);
    EXPECT_EQ(timer.sleepCalls()[0], 100U);
    EXPECT_EQ(timer.sleepCalls()[1], 100U);
    EXPECT_EQ(timer.sleepCalls()[2], 100U);
    // One frame per loop iteration expected
    EXPECT_EQ(driver.sentFrameCount(), 3U);
}

// ---------------------------------------------------------------------------
// MotorControl command reception
// ---------------------------------------------------------------------------

TEST(MotorEcuIntegration, IgnoresIncomingFrameWithWrongId)
{
    MockCanDriver       driver;
    EcuRunLimiterTimer  timer{99};
    MockRpmSensor       rpm_sensor{{800}};
    MockTempSensor      temp_sensor{{20}};
    MockMotorController motor_controller;

    CanFrame wrong{};
    wrong.id  = 0x999;
    wrong.dlc = 2;
    driver.enqueueReceiveFrame(wrong);

    MotorEcu ecu{driver, timer, rpm_sensor, temp_sensor, motor_controller};
    ecu.tick();

    EXPECT_EQ(motor_controller.callCount(), 0U);
}

TEST(MotorEcuIntegration, ForwardsValidCommandToMotorController)
{
    MockCanDriver       driver;
    EcuRunLimiterTimer  timer{99};
    MockRpmSensor       rpm_sensor{{800}};
    MockTempSensor      temp_sensor{{20}};
    MockMotorController motor_controller;

    // Build a valid MotorControl frame encoding target RPM = 3000
    vcansim_motor_control_t cmd{};
    cmd.target_rpm = vcansim_motor_control_target_rpm_encode(3000.0f);

    CanFrame frame{};
    frame.id  = VCANSIM_MOTOR_CONTROL_FRAME_ID;
    frame.dlc = VCANSIM_MOTOR_CONTROL_LENGTH;
    vcansim_motor_control_pack(frame.data.data(), &cmd, frame.data.size());

    driver.enqueueReceiveFrame(frame);

    MotorEcu ecu{driver, timer, rpm_sensor, temp_sensor, motor_controller};
    ecu.tick();

    ASSERT_EQ(motor_controller.callCount(), 1U);
    EXPECT_EQ(motor_controller.calls()[0], 3000U);
}