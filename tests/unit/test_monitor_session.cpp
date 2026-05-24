#include <gtest/gtest.h>

#include <sstream>

#include "mock_can_driver.h"
#include "monitor_core.h"
#include "monitor_session.h"

TEST(MonitorSessionTest, PumpOnceWritesSummaryForQueuedFrame)
{
    MockCanDriver driver;
    MonitorCore core(std::nullopt, std::nullopt);
    std::ostringstream output;
    MonitorSession session(driver, core, output);

    CanFrame frame{};
    frame.id = 0x123;
    frame.dlc = 2;
    frame.data[0] = 0xAB;
    frame.data[1] = 0xCD;
    driver.enqueueReceiveFrame(frame);

    EXPECT_TRUE(session.pumpOnce());
    EXPECT_NE(output.str().find("msg=MSG_0x123"), std::string::npos);
    EXPECT_EQ(driver.hasReceiveFrame(), false);
}

TEST(MonitorSessionTest, PumpOnceReturnsFalseWhenDriverQueueIsEmpty)
{
    MockCanDriver driver;
    MonitorCore core(std::nullopt, std::nullopt);
    std::ostringstream output;
    MonitorSession session(driver, core, output);

    EXPECT_FALSE(session.pumpOnce());
    EXPECT_TRUE(output.str().empty());
}