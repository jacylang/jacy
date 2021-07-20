#include "log/Logger.h"

namespace jc::log {
    const std::map<Config::LogLevel, std::string> Logger::levelNames = {
        {Config::LogLevel::Debug, "debug"},
        {Config::LogLevel::Info,  "info"},
        {Config::LogLevel::Warn,  "warn"},
        {Config::LogLevel::Error, "error"},
        {Config::LogLevel::Dev,   "[DEV]"}
    };

    const std::map<Config::LogLevel, Color> Logger::levelColors = {
        {Config::LogLevel::Debug, Color::Blue},
        {Config::LogLevel::Info,  Color::DarkGreen},
        {Config::LogLevel::Warn,  Color::Yellow},
        {Config::LogLevel::Error, Color::Red},
        {Config::LogLevel::Dev,   Color::Magenta},
    };
}
