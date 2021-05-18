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
const Logger & Logger::debug(Args && ...args) {
    return log(LogLevel::Debug, args...);
}

template<class ...Args>
const Logger & Logger::info(Args && ...args) {
    return log(LogLevel::Info, args...);
}

template<class ...Args>
const Logger & Logger::warn(Args && ...args) {
    return log(LogLevel::Warn, args...);
}

template<class ...Args>
const Logger & Logger::error(Args && ...args) {
    return log(LogLevel::Error, args...);
}

template<class ...Args>
const Logger & Logger::dev(Args && ...args) {
    return log(LogLevel::Dev, args...);
}

template<class Arg, class ...Args>
void Logger::devPanic(Arg && first, Args && ...other) {
    std::cout << levelColors.at(LogLevel::Error);
    std::cout << "[DEV PANIC]: " << std::forward<Arg>(first);
    ((std::cout << ' ' << std::forward<Args>(other)), ...);
    std::cout << Color::Reset;
    Logger::nl();

    throw common::Error("Stop after dev panic!");
}

template<class Arg, class... Args>
void Logger::devDebug(Arg && first, Args && ... other) {
    std::cout << levelColors.at(LogLevel::Debug) << "[DEV DEBUG]: " << Color::Reset << std::forward<Arg>(first);
    ((std::cout << ' ' << std::forward<Args>(other)), ...);
    Logger::nl();
}

template<class Arg, class ...Args>
const Logger & Logger::raw(Arg && first, Args && ...other) const {
    std::cout << std::forward<Arg>(first);
    ((std::cout << ' ' << std::forward<Args>(other)), ...);
    return *this;
}

template<class Arg, class ...Args>
void Logger::colorized(Color color, Arg && first, Args && ...other) {
    std::cout << colors.at(color); // Not putting it to `raw`, because it will be prefixed with white-space
    raw(first, other..., Color::Reset).nl();
}

template<class Arg, class ...Args>
void Logger::print(Arg && first, Args && ...other)  {
    std::cout << std::forward<Arg>(first);
    ((std::cout << ' ' << std::forward<Args>(other)), ...);
}

template<class Arg, class ...Args>
std::string Logger::format(Arg && first, Args && ...other) const {
    std::stringstream ss;
    ss << first;
    ((ss << other), ...);
    return ss.str();
}

template<class Arg, class ...Args>
const Logger & Logger::log(LogLevel level, Arg && first, Args && ...other) const {
    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(config.level)) {
        return *this;
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

    return *this;
}
