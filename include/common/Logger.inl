#include "Logger.h"

/**
 * std::ostream overloads
 */
template<class T>
std::ostream & operator<<(std::ostream & os, const std::vector<T> & vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
            os << vec.at(i);
            if (i < vec.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

template<class K, class V>
std::ostream & operator<<(std::ostream & os, const std::map<K, V> & map) {
    os << "{";
    for (auto it = map.begin(); it != map.end(); it++) {
        os << it->first << ": " << it->second;
        if (it != std::prev(map.end())) {
            os << ", ";
        }
    }
    os << "}";
    return os;
}

inline std::ostream & operator<<(std::ostream & os, Color color) {
    os << Logger::colors.at(color);
}

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

template<class ...Args>
void Logger::dev(Args && ...args) {
    log(LogLevel::Dev, args...);
}

template<class Arg, class ...Args>
void Logger::devPanic(Arg && first, Args && ...other) {
    std::cout << colors.at(Color::Red);
    std::cout << "[DEV PANIC]: " << std::forward<Arg>(first);
    ((std::cout << ' ' << std::forward<Args>(other)), ...);
    std::cout << Logger::ansiReset;
    Logger::nl();

    throw common::Error("Stop after dev panic!");
}

template<class Arg, class ...Args>
Logger & Logger::raw(Arg && first, Args && ...other) {
    std::cout << std::forward<Arg>(first);
    ((std::cout << ' ' << std::forward<Args>(other)), ...);
    return *this;
}

template<class Arg, class ...Args>
void Logger::colorized(Color color, Arg && first, Args && ...other) {
    std::cout << colors.at(color); // Not putting it to `raw`, because it will be prefixed with white-space
    raw(first, other..., ansiReset).nl();
}

template<class Arg, class ...Args>
void Logger::print(Arg && first, Args && ...other) {
    std::cout << std::forward<Arg>(first);
    ((std::cout << ' ' << std::forward<Args>(other)), ...);
}

template<class Arg, class ...Args>
std::string Logger::format(Arg && first, Args && ...other) {
    std::stringstream ss;
    ss << first;
    ((ss << other), ...);
    return ss.str();
}

template<class Arg, class ...Args>
void Logger::log(LogLevel level, Arg && first, Args && ...other) {
    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(config.level)) {
        return;
    }

    const auto & dev = level == LogLevel::Dev;

    if (config.printOwner or dev) {
        std::cout << owner << " ";
    }

    if (config.printLevel or dev) {
        if (config.colorize or dev) {
            std::cout << colors.at(levelColors.at(level));
        }
        std::cout << levelNames.at(level) << ": ";
        if (config.colorize or dev) {
            std::cout << Color::Reset;
        }
    }

    std::cout << std::forward<Arg>(first);

    ((std::cout << ' ' << std::forward<Args>(other)), ...);

    nl();
}
