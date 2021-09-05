#include "log/Logger.h"

namespace jc::log {
    Logger::Logger(const std::string &owner) : owner{std::move(owner)} {
        config.level = config::Config::getInstance().getLogLevel(owner);
    }

    // Constants //
    const std::map<LogLevel, std::string> Logger::levelNames = {
        {LogLevel::Debug, "debug"},
        {LogLevel::Info,  "info"},
        {LogLevel::Warn,  "warn"},
        {LogLevel::Error, "error"},
        {LogLevel::Dev,   "[DEV]"}
    };

    const std::map<LogLevel, Color> Logger::levelColors = {
        {LogLevel::Debug, Color::Blue},
        {LogLevel::Info,  Color::DarkGreen},
        {LogLevel::Warn,  Color::Yellow},
        {LogLevel::Error, Color::Red},
        {LogLevel::Dev,   Color::Magenta},
    };
}
