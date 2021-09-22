#include "log/Logger.h"

namespace jc::log {
    Logger::Logger(const std::string & owner) : owner {std::move(owner)} {
        config.level = config::Config::getInstance().getLogLevel(owner);
        config.devLogs = config::Config::getInstance().checkDevLog(owner);
    }

    // Constants //
    const std::map<Config::LogLevel, std::string> Logger::levelNames = {
        {Config::LogLevel::Debug, "debug"},
        {Config::LogLevel::Info,  "info"},
        {Config::LogLevel::Warn,  "warn"},
        {Config::LogLevel::Error, "error"},
    };

    const std::map<Config::LogLevel, Color> Logger::levelColors = {
        {Config::LogLevel::Debug, Color::Blue},
        {Config::LogLevel::Info,  Color::DarkGreen},
        {Config::LogLevel::Warn,  Color::Yellow},
        {Config::LogLevel::Error, Color::Red},
    };

    const Color Logger::devLogsColor = Color::Magenta;
    const std::string Logger::devLogName = "[DEV]";
}
