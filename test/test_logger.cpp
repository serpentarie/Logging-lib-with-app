#include <gtest/gtest.h>
#include "logger.h"
#include <fstream>
#include <string>
#include <sstream>

class LoggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_file = "test_log.txt";
    }

    void TearDown() override
    {
        std::remove(test_file.c_str());
    }

    std::string readFile()
    {
        std::ifstream ifs(test_file);
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        return buffer.str();
    }

    std::string test_file;
};

TEST_F(LoggerTest, FileLogger_WritesCorrectFormat)
{
    Logger logger(test_file, LogLevel::INFO);
    logger.log("Info", LogLevel::INFO);

    std::string content = readFile();
    EXPECT_FALSE(content.empty());
    EXPECT_TRUE(content.find("[INFO] Info") != std::string::npos);
}

TEST_F(LoggerTest, FileLogger_RespectsLogLevel)
{
    Logger logger(test_file, LogLevel::WARNING);
    logger.log("Does not appear", LogLevel::INFO);
    logger.log("Do appear", LogLevel::WARNING);

    std::string content = readFile();
    EXPECT_TRUE(content.find("Does not appear") == std::string::npos);
    EXPECT_TRUE(content.find("[WARNING] Do appear") != std::string::npos);
}

TEST_F(LoggerTest, SetLogLevel_ChangesLevel)
{
    Logger logger(test_file, LogLevel::ERROR);
    logger.log("Does not appear", LogLevel::WARNING);
    logger.setLogLevel(LogLevel::WARNING);
    logger.log("Do appear", LogLevel::WARNING);

    std::string content = readFile();
    EXPECT_TRUE(content.find("Does not appear") == std::string::npos);
    EXPECT_TRUE(content.find("[WARNING] Do appear") != std::string::npos);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}