#include <gtest/gtest.h>

#include "sim_engine.h"

// ---------------------------------------------------------------------------
// RPM ramp behavior
// ---------------------------------------------------------------------------

TEST(SimEngineRpm, StartsAtDefaultRpm)
{
    SimEngine engine;
    EXPECT_EQ(engine.readRpm(), 800U);
}

TEST(SimEngineRpm, RampsUpTowardTarget)
{
    SimEngine engine;
    engine.setTargetRpm(1000U);

    EXPECT_EQ(engine.readRpm(), 900U);
    EXPECT_EQ(engine.readRpm(), 1000U);
}

TEST(SimEngineRpm, ClampsAtTarget)
{
    SimEngine engine;
    engine.setTargetRpm(1000U);

    engine.readRpm(); // 900
    engine.readRpm(); // 1000
    engine.readRpm(); // still 1000

    EXPECT_EQ(engine.readRpm(), 1000U);
}

TEST(SimEngineRpm, RampsDownTowardTarget)
{
    SimEngine engine;
    engine.setTargetRpm(2000U);
    // ramp up to 2000 first
    while (engine.readRpm() < 2000U) {}

    engine.setTargetRpm(1800U);
    EXPECT_EQ(engine.readRpm(), 1900U);
    EXPECT_EQ(engine.readRpm(), 1800U);
}

TEST(SimEngineRpm, RampsDownToZero)
{
    SimEngine engine;
    engine.setTargetRpm(0U);

    // from 800 down to 0 in steps of 100
    for (uint16_t expected = 700U; expected > 0U; expected -= 100U)
        EXPECT_EQ(engine.readRpm(), expected);

    EXPECT_EQ(engine.readRpm(), 0U);
    EXPECT_EQ(engine.readRpm(), 0U); // stays at 0
}

// ---------------------------------------------------------------------------
// Temperature
// ---------------------------------------------------------------------------

TEST(SimEngineTemp, ReturnsFixedTemperature)
{
    SimEngine engine;
    EXPECT_EQ(engine.readTemp(), 70);
    EXPECT_EQ(engine.readTemp(), 70); // unchanged across calls
}