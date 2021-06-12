#include "common/Logger.h"

namespace jc::common {
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

    const std::map<Color, std::string> Logger::unixColors = {
        {Color::Black,     "30"},
        {Color::DarkBlue,  "34"},
        {Color::DarkGreen, "32"},
        {Color::LightBlue, "36"},
        {Color::DarkRed,   "31"},
        {Color::Magenta,   "35"},
        {Color::Orange,    "33"},
        {Color::LightGray, "37"},
        {Color::Gray,      "90"},
        {Color::Blue,      "94"},
        {Color::Green,     "92"},
        {Color::Cyan,      "96"},
        {Color::Red,       "91"},
        {Color::Pink,      "95"},
        {Color::Yellow,    "93"},
        {Color::White,     "97"},

        {Color::Reset,     "0"},
    };
}
