#include "log/Logger.h"

// Basic Interface //
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
    if (not config.devLogs) {
        return *this;
    }
    out(std::cout, devLogsColor, devLogName, Color::Reset, ": ", std::forward<Args>(args)...);
    nl();
    return *this;
}

template<class ...Args>
const Logger & Logger::raw(Args && ...args) const {
    print(std::forward<Args>(args)...);
    return *this;
}

// Static Interface //
template<class ...Args>
void Logger::print(Args && ...args)  {
    out(std::cout, std::forward<Args>(args)...);
}

// DEV //
template<class ...Args>
void Logger::devDebug(Args && ...args) {
    // Note!!!: `devDebug` only works if `dev-full` option enabled
    if (not Config::getInstance().checkDevFull()) {
        return;
    }
    out(
        std::cout,
        devLogsColor,
        devLogName,
        ": ",
        Color::Reset,
        std::forward<Args>(args)...
    );
    nl();
}

// Pretty printing //
template<class ...Args>
void Logger::colorized(Color color, Args && ...args) {
    out(std::cout, std::forward<Color>(color), std::forward<Args>(args)..., Color::Reset);
    nl();
}

template<class ...Args>
void Logger::printTitleDev(Args && ...args) {
    // Note!!!: `printTitleDev` works only if `dev` option enabled
    if (not Config::getInstance().checkDevMode()) {
        return;
    }

    const auto & title = fmt(std::forward<Args>(args)...);
    if (title.size() > wrapLen + 2) {
        // FIXME: Yeah, wtf? WE PANIC INSIDE LOGGER!!!
        devPanic("Too long message in `Logger::printTitleDev`");
    }
    const auto indentSize = (wrapLen - title.size()) / 2 - 1;
    std::string indent = std::string(indentSize, '=');

    std::cout << indent << (indentSize % 2 == 1 ? "= " : " ") << title << " " << indent << std::endl;
}

// Details //
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
