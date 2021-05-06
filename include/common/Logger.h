#ifndef JACY_LOGGER_H
#define JACY_LOGGER_H

#include <cstdint>
#include <iostream>
#include <utility>
#include <map>
#include <string>
#include <sstream>

namespace jc::common {
    enum class Color {
        Red,
        Green,
        Blue,
        Yellow,
        Magenta,
        Cyan,
    };

    // LogLevel //
    enum class LogLevel : uint8_t {
        Verbose,
        Debug,
        Info,
        Warn,
        Error,
    };

    struct LoggerConfig {
        LogLevel level{LogLevel::Debug};
        bool logOwner{true};
        bool logLevel{true};
        bool colorize{true};
    };

    class Logger {
    public:
        Logger(std::string owner, const LoggerConfig & config) : owner(std::move(owner)), config(config) {}
        virtual ~Logger() = default;

        template<class ...Args>
        void verbose(Args && ...args);

        template<class ...Args>
        void debug(Args && ...args);

        template<class ...Args>
        void info(Args && ...args);

        template<class ...Args>
        void warn(Args && ...args);

        template<class ...Args>
        void error(Args && ...args);

    private:
        std::string owner;
        LoggerConfig config;

        template<class Arg, class ...Args>
        void log(LogLevel level, Arg && first, Args && ...other);

        static const std::map<LogLevel, std::string> levelNames;
        static const std::map<LogLevel, Color> levelColors;
        static const std::map<Color, std::string> colors;
        static const std::string ansiReset;
    };

    #include "common/Logger.inl"
}

#endif // JACY_LOGGER_H
