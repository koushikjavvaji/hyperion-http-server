#include <hyperion/logger.h>
#include <iostream>
#include <chrono>

namespace hyperion
{
    void Logger::log(LogLevel level, const std::string &msg)
    {
        const char *label = "INFO";
        if (level == LogLevel::DEBUG)
            label = "DEBUG";
        if (level == LogLevel::WARN)
            label = "WARN";
        if (level == LogLevel::ERROR)
            label = "ERROR";
        std::cout << "[" << label << "] " << msg << std::endl;
    }

    void Logger::info(const std::string &msg) { log(LogLevel::INFO, msg); }
    void Logger::error(const std::string &msg) { log(LogLevel::ERROR, msg); }
}