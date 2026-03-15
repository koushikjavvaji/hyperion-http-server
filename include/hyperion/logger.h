#pragma once
#include <string>

namespace hyperion
{
    enum class LogLevel
    {
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    class Logger
    {
    public:
        static void log(LogLevel level, const std::string &msg);
        static void info(const std::string &msg);
        static void error(const std::string &msg);
    };
}