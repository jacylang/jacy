#include "Logger.h"


template<class ...Args>
void Logger::verbose(Args && ...args) {
    log(LogLevel::Verbose, args...);
}

template<class ...Args>
void Logger::debug(Args && ...args) {
    log(LogLevel::Debug, args...);
}

template<class ...Args>
void Logger::info(Args && ...args) {
    log(LogLevel::Info, args...);
}

template<class ...Args>
void Logger::warn(Args && ...args) {
    log(LogLevel::Warn, args...);
}

template<class ...Args>
void Logger::error(Args && ...args) {
    log(LogLevel::Error, args...);
}

template<class Arg, class ...Args>
void Logger::log(LogLevel level, Arg && first, Args && ...other) {
    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(config.level)) {
        return;
    }

    if (config.logOwner) {
        std::cout << owner << " ";
    }

    if (config.logLevel) {
        if (config.colorize) {
            std::cout << colors.at(levelColors.at(level));
        }
        std::cout << levelNames.at(level) << ": ";
        if (config.colorize) {
            std::cout << Logger::ansiReset;
        }
    }

    std::cout << std::forward<Arg>(first);

    ((std::cout << ' ' << std::forward<Args>(other)), ...);

    std::cout << std::endl;
}
