#include "logger.h"
#include <iostream>
#include <string>
#include <sstream>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Enter: " << argv[0] << " [host] [port] [default_level]" << std::endl;
        std::cerr << "Level variety: ERROR, WARNING, INFO" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);
    std::string level_str = argv[3];
    LogLevel default_level;
    if (level_str == "ERROR")
        default_level = LogLevel::ERROR;
    else if (level_str == "WARNING")
        default_level = LogLevel::WARNING;
    else if (level_str == "INFO")
        default_level = LogLevel::INFO;
    else
    {
        std::cerr << "Invalid level: " << level_str << std::endl;
        return 1;
    }

    Logger logger(host, port, default_level);

    std::cout << "Enter '[message] [level]' or 'exit' to quit" << std::endl;
    while (true)
    {
        std::string input;
        std::getline(std::cin, input);
        if (input == "exit")
            break;

        std::istringstream iss(input);
        std::string msg, level_str;
        std::string token;
        while (iss >> token)
        {
            if (!level_str.empty())
            {
                msg += (msg.empty() ? "" : " ") + level_str;
            }
            level_str = token;
        }
        if (msg.empty())
        {
            msg = level_str;
            level_str = "";
        }

        LogLevel level = default_level;
        if (!level_str.empty())
        {
            if (level_str == "ERROR")
                level = LogLevel::ERROR;
            else if (level_str == "WARNING")
                level = LogLevel::WARNING;
            else if (level_str == "INFO")
                level = LogLevel::INFO;
            else
                msg += (msg.empty() ? "" : " ") + level_str;
        }

        logger.log(msg, level);
    }
}