#include "common/Logger.h"

namespace jc::common {
    const std::map<LogLevel, std::string> Logger::levelNames = {
        {LogLevel::Verbose, "verbose"},
        {LogLevel::Debug, "debug"},
        {LogLevel::Info, "info"},
        {LogLevel::Warn, "warn"},
        {LogLevel::Error, "error"},
        {LogLevel::Dev, "[DEV]"}
    };

    const std::map<LogLevel, Color> Logger::levelColors = {
        {LogLevel::Verbose, Color::Magenta},
        {LogLevel::Debug, Color::Blue},
        {LogLevel::Info, Color::Green},
        {LogLevel::Warn, Color::Yellow},
        {LogLevel::Error, Color::Red},
        {LogLevel::Dev, Color::Magenta},
    };

    const std::map<Color, std::string> Logger::colors = {
        {Color::Red, "\033[1;31m"},
        {Color::Green, "\033[1;32m"},
        {Color::Blue, "\033[1;34m"},
        {Color::Yellow, "\033[1;33m"},
        {Color::Magenta, "\033[1;35m"},
        {Color::Cyan, "\033[1;36m"},
        {Color::Reset, "\033[1;0m"},
    };
}
