#include "logger.h"
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <csignal>

struct Stats
{
    size_t total = 0;
    size_t errors = 0;
    size_t warnings = 0;
    size_t infos = 0;
    size_t last_hour = 0;
    size_t min_len = SIZE_MAX;
    size_t max_len = 0;
    double avg_len = 0.0;

    bool operator==(const Stats &other) const
    {
        return total == other.total &&
               errors == other.errors &&
               warnings == other.warnings &&
               infos == other.infos &&
               last_hour == other.last_hour &&
               min_len == other.min_len &&
               max_len == other.max_len &&
               avg_len == other.avg_len;
    }
    bool operator!=(const Stats &other) const
    {
        return !(*this == other);
    }
};

std::string extract_time_str(const std::string &log_msg)
{
    size_t start = log_msg.find('[');
    if (start == std::string::npos)
        return "";
    size_t end = log_msg.find(']', start + 1);
    if (end == std::string::npos)
        return "";
    return log_msg.substr(start + 1, end - start - 1);
}

std::string extract_level_str(const std::string &log_msg, size_t after_pos)
{
    size_t start = log_msg.find('[', after_pos);
    if (start == std::string::npos)
        return "";
    size_t end = log_msg.find(']', start + 1);
    if (end == std::string::npos)
        return "";
    return log_msg.substr(start + 1, end - start - 1);
}

std::string extract_message(const std::string &log_msg, size_t after_pos)
{
    if (after_pos == std::string::npos || after_pos + 2 >= log_msg.size())
        return "";
    return log_msg.substr(after_pos + 2);
}

std::time_t parse_time(const std::string &time_str)
{
    std::tm tm = {};
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm, "%d-%m-%Y %H:%M:%S");
    if (ss.fail())
    {
        return 0;
    }
    return std::mktime(&tm);
}

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

void print_stats(const Stats &s)
{
    std::cout << "Total messages: " << s.total << std::endl;
    std::cout << "Errors: " << s.errors << std::endl;
    std::cout << "Warnings: " << s.warnings << std::endl;
    std::cout << "Infos: " << s.infos << std::endl;
    std::cout << "Messages in last hour: " << s.last_hour << std::endl;
    std::cout << "Min len: " << (s.min_len == SIZE_MAX ? 0 : s.min_len) << std::endl;
    std::cout << "Max len: " << s.max_len << std::endl;
    std::cout << "Avg len: " << s.avg_len << std::endl;
    std::cout << std::endl;
}

std::atomic<bool> running(true);

void signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        running.store(false);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cerr << "Enter: " << argv[0] << " [host] [port] [N] [T]" << std::endl;
        std::cerr << "host: " << std::endl;
        std::cerr << "port: " << std::endl;
        std::cerr << "N: " << std::endl;
        std::cerr << "T: " << std::endl;
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);
    size_t N = std::stoul(argv[3]);
    int T = std::stoi(argv[4]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 1;
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (host == "0.0.0.0")
    {
        servaddr.sin_addr.s_addr = INADDR_ANY;
    }
    else if (inet_pton(AF_INET, host.c_str(), &servaddr.sin_addr) <= 0)
    {
        perror("Invalid addres");
        close(sockfd);
        return 1;
    }

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    std::signal(SIGINT, signal_handler);

    std::vector<std::time_t> timestamps;
    size_t sum_lengths = 0;
    size_t min_length = SIZE_MAX;
    size_t max_length = 0;
    size_t counts[3] = {0, 0, 0};
    size_t total = 0;
    std::mutex mut;
    Stats last_printed;
    std::atomic<bool> running(true);

    std::thread timer_thread([&]()
                             {
                                 while (running.load())
                                 {
                                     std::this_thread::sleep_for(std::chrono::seconds(T));
                                     std::unique_lock<std::mutex> lk(mut);
                                     std::time_t now = std::time(nullptr);
                                     auto it = std::lower_bound(timestamps.begin(), timestamps.end(), now - 3600);
                                     size_t last_h = timestamps.end() - it;
                                     double avg = (total > 0) ? static_cast<double>(sum_lengths) / total : 0.0;
                                     Stats current{
                                         total,
                                         counts[0],
                                         counts[1],
                                         counts[2],
                                         last_h,
                                         (total == 0) ? 0 : min_length,
                                         max_length,
                                         avg};
                                     if (current != last_printed)
                                     {
                                         print_stats(current);
                                         last_printed = current;
                                     }
                                 } });

    char buffer[4096];
    while (running.load())
    {
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);
        ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&cliaddr, &len);
        if (recv_len < 0)
        {
            if (!running.load())
                break;
            perror("Recv failed");
            continue;
        }
        buffer[recv_len] = '\0';
        std::string log_msg(buffer);

        std::cout << log_msg << std::endl;

        std::string time_str = extract_time_str(log_msg);
        size_t after_time = log_msg.find(']', 0) + 1;
        std::string level_str = extract_level_str(log_msg, after_time);
        size_t after_level = log_msg.find(']', after_time) + 1;
        std::string message = extract_message(log_msg, after_level);

        std::time_t ts = parse_time(time_str);
        if (ts == 0)
        {
            std::cerr << "Failed parsing time: " << time_str << std::endl;
            continue;
        }
        LogLevel level = parse_level(level_str);

        size_t msg_len = message.length();

        std::lock_guard<std::mutex> lk(mut);
        timestamps.push_back(ts);
        counts[static_cast<int>(level)]++;
        sum_lengths += msg_len;
        min_length = std::min(min_length, msg_len);
        max_length = std::max(max_length, msg_len);
        total++;

        if (N > 0 && total % N == 0)
        {
            std::time_t now = std::time(nullptr);
            auto it = std::lower_bound(timestamps.begin(), timestamps.end(), now - 3600);
            size_t last_h = timestamps.end() - it;
            double avg = static_cast<double>(sum_lengths) / total;
            Stats current{
                total,
                counts[0],
                counts[1],
                counts[2],
                last_h,
                (total == 0) ? 0 : min_length,
                max_length,
                avg};
            print_stats(current);
            last_printed = current;
        }
    }
    running.store(false);
    timer_thread.join();
    close(sockfd);
}