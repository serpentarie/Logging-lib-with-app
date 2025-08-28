#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <memory>
#include <netinet/in.h>

enum class LogLevel
{
    ERROR,
    WARNING,
    INFO
};

class ILogWriter
{
public:
    virtual void write(const std::string &msg) = 0;
    virtual ~ILogWriter() = default;
};

class FileLogWriter : public ILogWriter
{
public:
    FileLogWriter(const std::string &filename, bool append = true);
    void write(const std::string &msg) override;

private:
    std::ofstream ofs;
};

class SocketLogWriter : public ILogWriter
{
public:
    SocketLogWriter(const std::string &host, int port);
    ~SocketLogWriter();
    void write(const std::string &msg) override;

public:
    int sockfd;
    struct sockaddr_in servaddr;
};

class Logger
{
public:
    Logger(const std::string &filename, LogLevel defaultLevel, bool append = true);
    Logger(const std::string &host, int port, LogLevel defaultLevel);

    void log(const std::string &message, LogLevel level);
    void setLogLevel(LogLevel level);

private:
    std::unique_ptr<ILogWriter> writer;
    LogLevel currentLevel;

    std::string getCurrentTime() const;
    std::string levelToString(LogLevel level) const;
};

#endif