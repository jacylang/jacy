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

template<class K, class V>
inline std::ostream & operator<<(std::ostream & os, const std::unordered_map<K, V> & map) {
    os << "{";
    size_t i = 0;
    for (const auto & el : map) {
        os << el.first << ": " << el.second;
        if (i != map.size()) {
            os << ", ";
        }
        i++;
    }
    os << "}";
    return os;
}

std::ostream & operator<<(std::ostream & os, Color color) {
#if defined(WIN)
    static const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    const auto colorNum = static_cast<uint8_t>(color);
    SetConsoleTextAttribute(handle, colorNum);
#elif defined(UNIX)
    os << "\x1b[" << Logger::unixColors.at(color) << "m";
#endif
    return os;
}

inline std::ostream & operator<<(std::ostream & os, Indent indent) {
    std::cout << utils::str::repeat(Indent::indentString, indent.inner);
    return os;
}

template<class ...Args>
const Logger & Logger::debug(Args && ...args) const {
    return log(Config::LogLevel::Debug, std::forward<Args>(args)...);
}

template<class ...Args>
const Logger & Logger::info(Args && ...args) const {
    return log(Config::LogLevel::Info, std::forward<Args>(args)...);
}

template<class ...Args>
const Logger & Logger::warn(Args && ...args) const {
    return log(Config::LogLevel::Warn, std::forward<Args>(args)...);
}

template<class ...Args>
const Logger & Logger::error(Args && ...args) const {
    return log(Config::LogLevel::Error, std::forward<Args>(args)...);
}

template<class ...Args>
const Logger & Logger::dev(Args && ...args) const {
    return log(Config::LogLevel::Dev, std::forward<Args>(args)...);
}

template<class Arg, class ...Args>
void Logger::devPanic(Arg && first, Args && ...other) {
    std::cout << levelColors.at(Config::LogLevel::Error);
    std::cout << "[DEV PANIC]: " << std::forward<Arg>(first);
    ((std::cout << ' ' << std::forward<Args>(other)), ...);
    std::cout << Color::Reset;
    Logger::nl();

    throw common::Error("Stop after dev panic!");
}

template<class Arg, class... Args>
void Logger::devDebug(Arg && first, Args && ... other) {
    if (not Config::getInstance().checkLogLevel(Config::LogLevel::Dev)) {
        return;
    }
    std::cout << levelColors.at(Config::LogLevel::Dev) << "[DEV]: " << Color::Reset << std::forward<Arg>(first);
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
    std::cout << color; // Not putting it to `raw`, because it will be prefixed with white-space
    raw(first, other..., Color::Reset).nl();
}

template<class Arg, class ...Args>
void Logger::printTitleDev(Arg && first, Args && ...other) {
    if (not Config::getInstance().checkLogLevel(Config::LogLevel::Dev)) {
        return;
    }

    std::stringstream ss;
    ss << std::forward<Arg>(first);
    ((ss << ' ' << std::forward<Args>(other)), ...);

    const auto & title = ss.str();
    if (title.size() > wrapLen + 2) {
        // FIXME: Yeah, wtf? WE PANIC INSIDE LOGGER!!!
        devPanic("Too long message in `Logger::printTitleDev`");
    }
    const auto indentSize = (wrapLen - title.size()) / 2 - 1;
    std::string indent = utils::str::repeat("=", indentSize);

    std::cout << indent << (indentSize % 2 == 1 ? "= " : " ") << title << " " << indent << std::endl;
}

template<class Arg, class ...Args>
void Logger::print(Arg && first, Args && ...other)  {
    std::cout << std::forward<Arg>(first);
    ((std::cout << ' ' << std::forward<Args>(other)), ...);
}

template<class Arg, class ...Args>
std::string Logger::format(Arg && first, Args && ...other) {
    std::stringstream ss;
    ss << first;
    ((ss << ' ' << other), ...);
    return ss.str();
}

template<class First, class ...Rest>
const Logger & Logger::log(Config::LogLevel level, First && first, Rest && ...rest) const {
    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(config.level)) {
        return *this;
    }

    printInfo(level);
    logHelper(std::forward<First>(first));
    logHelper(std::forward<Rest>(rest)...);
    nl();

    return *this;
}

template<class First>
const Logger & Logger::log(Config::LogLevel level, First && first) const {
    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(config.level)) {
        return *this;
    }

    printInfo(level);
    logHelper(std::forward<First>(first));
    nl();

    return *this;
}

