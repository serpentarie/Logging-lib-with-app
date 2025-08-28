#include "logger.h"

int main()
{
    Logger logger("app.log", LogLevel::WARNING);

    logger.log("Information", LogLevel::INFO); // не запишется
    logger.log("Warning alarm", LogLevel::WARNING);
    logger.log("UGH", LogLevel::ERROR);

    logger.setLogLevel(LogLevel::INFO);
    logger.log("Information", LogLevel::INFO); // теперь инфо пишется
}