#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "csv_logger.h"

TEST(CsvLoggerTest, WritesHeaderAndRows)
{
    namespace fs = std::filesystem;
    fs::path tmp = "data/test_csv_logger";
    fs::remove_all(tmp);
    CsvLogger logger(tmp);

    logger.writeRow("MSG_TEST", {"timestamp", "frame_id", "value"}, {"1", "0x100", "42"});
    logger.writeRow("MSG_TEST", {"timestamp", "frame_id", "value"}, {"2", "0x100", "43"});

    fs::path f = tmp / "MSG_TEST.csv";
    ASSERT_TRUE(fs::exists(f));

    std::ifstream in(f);
    std::string line;
    std::getline(in, line);
    EXPECT_EQ(line, "timestamp,frame_id,value");
    std::getline(in, line);
    EXPECT_EQ(line, "1,0x100,42");
    std::getline(in, line);
    EXPECT_EQ(line, "2,0x100,43");
    fs::remove_all(tmp);
}

TEST(CsvLoggerTest, EscapesCommaAndQuotes)
{
    namespace fs = std::filesystem;
    fs::path tmp = "data/test_csv_logger_escape";
    fs::remove_all(tmp);
    CsvLogger logger(tmp);

    logger.writeRow("MSG_ESC", {"name", "value"}, {"plain", "a,b\"c"});

    fs::path f = tmp / "MSG_ESC.csv";
    ASSERT_TRUE(fs::exists(f));

    std::ifstream in(f);
    std::string line;
    std::getline(in, line);
    EXPECT_EQ(line, "name,value");
    std::getline(in, line);
    EXPECT_EQ(line, "plain,\"a,b\"\"c\"");
    fs::remove_all(tmp);
}
