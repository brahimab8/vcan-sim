#include <gtest/gtest.h>

#include "mock_can_driver.h"
#include "stop_timer.h"
#include "sim_engine.h"
#include "motor_ecu.h"
#include "vcansim.h"

// Integration test: send a MotorControl command and verify the SimEngine
// ramps the reported RPM toward the target via repeated ticks.

TEST(MotorControlIntegration, RampsRpmToTarget)
{
    MockCanDriver driver;
    EcuRunLimiterTimer timer{99};
    SimEngine engine;

    MotorEcu ecu{driver, timer, engine, engine, engine};

    // Build and enqueue MotorControl -> target RPM = 3000
    vcansim_motor_control_t cmd{};
    cmd.target_rpm = vcansim_motor_control_target_rpm_encode(3000.0f);

    CanFrame frame{};
    frame.id  = VCANSIM_MOTOR_CONTROL_FRAME_ID;
    frame.dlc = VCANSIM_MOTOR_CONTROL_LENGTH;
    vcansim_motor_control_pack(frame.data.data(), &cmd, frame.data.size());

    driver.enqueueReceiveFrame(frame);

    // First tick: ECU will send a status frame (old RPM) then process the command
    ecu.tick();

    // Run additional ticks to allow the SimEngine to ramp toward the target.
    // kRpmStep is 100, delta from 800 -> 3000 is 2200 => 22 steps; 25 ticks is safe.
    for (int i = 0; i < 25; ++i) {
        ecu.tick();
    }

    ASSERT_GE(driver.sentFrameCount(), 1u);
    const CanFrame& last = driver.sentFrames().back();

    vcansim_motor_status_t msg{};
    ASSERT_EQ(vcansim_motor_status_unpack(&msg, last.data.data(), last.dlc), 0);
    float rpm = vcansim_motor_status_rpm_decode(msg.rpm);

    // Expect the RPM to be close to the target (allow some tolerance)
    EXPECT_GE(rpm, 2900.0f);
}
