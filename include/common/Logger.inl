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
    return os << "]";
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
    return os << "}";
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
    return os << "}";
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

inline std::ostream & operator<<(std::ostream & os, const dt::none_t&) {
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

template<class ...Args>
void Logger::devPanic(Args && ...args) {
    out(
        std::cout,
        levelColors.at(Config::LogLevel::Error),
        "[DEV PANIC]: ",
        std::forward<Args>(args)...,
        Color::Reset
    );
    nl();

    throw common::Error("Stop after dev panic!");
}

template<class ...Args>
void Logger::devDebug(Args && ...args) {
    if (not Config::getInstance().checkLogLevel(Config::LogLevel::Dev)) {
        return;
    }
    out(
        std::cout,
        levelColors.at(Config::LogLevel::Dev),
        "[DEV]: ",
        Color::Reset,
        std::forward<Args>(args)...
    );
    nl();
}

template<class ...Args>
const Logger & Logger::raw(Args && ...args) const {
    print(std::forward<Args>(args)...);
    return *this;
}

template<class Arg, class ...Args>
const Logger & Logger::tableRow(TC<Arg> && first, TC<Args> && ...rest) const {
    raw("| ");
    tableRow(std::forward<TC<Arg>>(first));
    tableRow(std::forward<TC<Args>>(rest)...);
    nl();
    return *this;
}

template<class Arg>
const Logger & Logger::tableRow(TC<Arg> && arg) const {
    raw("| ");
    arg.print(std::cout);
    raw(" |");
    return *this;
}

template<class ...Args>
void Logger::colorized(Color color, Args && ...args) {
    out(std::cout, std::forward<Color>(color), std::forward<Args>(args)..., Color::Reset);
    nl();
}

template<class ...Args>
void Logger::printTitleDev(Args && ...args) {
    if (not Config::getInstance().checkLogLevel(Config::LogLevel::Dev)) {
        return;
    }

    const auto & title = format(std::forward<Args>(args)...);
    if (title.size() > wrapLen + 2) {
        // FIXME: Yeah, wtf? WE PANIC INSIDE LOGGER!!!
        devPanic("Too long message in `Logger::printTitleDev`");
    }
    const auto indentSize = (wrapLen - title.size()) / 2 - 1;
    std::string indent = utils::str::repeat("=", indentSize);

    std::cout << indent << (indentSize % 2 == 1 ? "= " : " ") << title << " " << indent << std::endl;
}

template<class ...Args>
void Logger::print(Args && ...args)  {
    out(std::cout, std::forward<Args>(args)...);
}

template<class ...Args>
std::string Logger::format(Args && ...args) {
    std::stringstream ss;
    out(ss, std::forward<Args>(args)...);
    return ss.str();
}

template<class ...Rest>
const Logger & Logger::log(Config::LogLevel level, Rest && ...rest) const {
    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(config.level)) {
        return *this;
    }

    printInfo(level);
    out(std::cout, std::forward<Rest>(rest)...);
    nl();

    return *this;
}
