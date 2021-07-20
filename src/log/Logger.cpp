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
