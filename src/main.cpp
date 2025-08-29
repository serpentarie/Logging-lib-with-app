#include "logger.h"
#include <iostream>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <sstream>

LogLevel parse_level(const std::string &level_str)
{
    if (level_str == "ERROR")
        return LogLevel::ERROR;
    if (level_str == "WARNING")
        return LogLevel::WARNING;
    if (level_str == "INFO")
        return LogLevel::INFO;
    return LogLevel::INFO;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Enter: " << argv[0] << " [filename] [default level]" << std::endl;
        std::cerr << "Level variety: ERROR, WARNING, INFO" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string default_level_str = argv[2];
    LogLevel default_level = parse_level(default_level_str);
    if (default_level == LogLevel::INFO && default_level_str != "INFO")
    {
        std::cerr << "Invalid defult level: " << default_level_str << std::endl;
    }

    Logger logger(filename, default_level);

    std::queue<std::pair<std::string, LogLevel>> log_queue;
    std::mutex mut;
    std::condition_variable cv;
    std::atomic<bool> done(false);
    std::thread worker([&]()
                       {
        while (true){
            std::unique_lock<std::mutex> lk(mut);
            cv.wait(lk, [&]{ return !log_queue.empty() || done.load();});
            if (done.load() && log_queue.empty()) {
                break;
            }
            auto item = log_queue.front();
            log_queue.pop();
            lk.unlock();
            logger.log(item.first, item.second);
        } });

    std::cout << "Enter '[message] + [level of importance]' or 'exit' to exit" << std::endl;
    while (true)
    {
        std::string input;
        std::getline(std::cin, input);
        if (input.empty())
            continue;
        if (input == "exit")
            break;

        std::istringstream iss(input);
        std::string msg, token, posible_level;
        while (iss >> token)
        {
            if (!posible_level.empty())
            {
                msg += " " + posible_level;
            }
            posible_level = token;
        }
        if (!msg.empty())
        {
            msg = msg.substr(1);
        }
        else
        {
            msg = posible_level;
            posible_level.clear();
        }

        LogLevel level = default_level;
        if (!posible_level.empty())
        {
            LogLevel parsed = parse_level(posible_level);
            if (parsed != LogLevel::INFO || posible_level == "INFO")
            {
                level = parsed;
            }
            else
            {
                msg += " " + posible_level;
            }
        }

        {
            std::lock_guard<std::mutex> lk(mut);
            log_queue.push({msg, level});
        }
        cv.notify_one();
    }
    done.store(true);
    cv.notify_one();
    worker.join();
}