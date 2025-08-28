#include "logger.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <iomanip>

FileLogWriter::FileLogWriter(const std::string &filename, bool append)
{
    if (append)
    {
        ofs.open(filename, std::ios::app);
    }
    else
    {
        ofs.open(filename, std::ios::out | std::ios::trunc);
    }

    if (!ofs)
    {
        std::cerr << "Error opening file: " << filename << std::endl;
    }
}

void FileLogWriter::write(const std::string &msg)
{
    if (ofs.is_open())
    {
        ofs << msg << std::endl;
    }
}

SocketLogWriter::SocketLogWriter(const std::string &host, int port)
{
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &servaddr.sin_addr) <= 0)
    {
        perror("Host error");
        close(sockfd);
        sockfd = -1;
    }
}

SocketLogWriter::~SocketLogWriter()
{
    if (sockfd >= 0)
    {
        close(sockfd);
    }
}

void SocketLogWriter::write(const std::string &msg)
{
    if (sockfd >= 0)
    {
        sendto(sockfd, msg.c_str(), msg.length(), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }
}

Logger::Logger(const std::string &filename, LogLevel defaultLevel, bool append) : writer(std::make_unique<FileLogWriter>(filename, append)), currentLevel(defaultLevel) {}

Logger::Logger(const std::string &host, int port, LogLevel defaultLevel) : writer(std::make_unique<SocketLogWriter>(host, port)), currentLevel(defaultLevel) {}

void Logger::log(const std::string &message, LogLevel level)
{
    if (level > currentLevel)
    {
        return;
    }
    std::string logMsg = "[" + getCurrentTime() + "][" + levelToString(level) + "] " + message;
    writer->write(logMsg);
}

void Logger::setLogLevel(LogLevel level)
{
    currentLevel = level;
}

std::string Logger::getCurrentTime() const
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&in_time_t);

    std::stringstream ss;
    ss << std::put_time(&local_tm, "%d-%m-%Y %H:%M:%S");
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::INFO:
        return "INFO";
    default:
        return "UNKNOWN";
    }
}