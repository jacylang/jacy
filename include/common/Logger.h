#ifndef JACY_COMMON_LOGGER_H
#define JACY_COMMON_LOGGER_H

#include <cstdint>
#include <iostream>
#include <utility>
#include <map>
#include <string>
#include <sstream>
#include <sstream>
#include <vector>

#include "common/Error.h"
#include "common/Config.h"
#include "utils/str.h"

#if defined(__unix__) \
    || defined(__unix) \
    || defined(__linux__) \
    || defined(__APPLE__) \
    || defined(__MACH__) \
    || defined(__MINGW32__) \
    || defined(__MINGW64__) \
    || defined(__GNUC__)
#define UNIX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define WIN
#else
#error "Unknown platform"
#endif // OS definitions

#ifdef WIN
#include <windows.h>
#endif // WIN

namespace jc::common {
    // Note: Discriminants are Window only (!)
    enum class Color : uint8_t {
        Black           = 0,
        DarkBlue        = 1,
        DarkGreen       = 2,
        LightBlue       = 3,
        DarkRed         = 4,
        Magenta         = 5,
        Orange          = 6, // Actually, may looks like brown
        LightGray       = 7,
        Gray            = 8,
        Blue            = 9,
        Green           = 10,
        Cyan            = 11,
        Red             = 12,
        Pink            = 13,
        Yellow          = 14,
        White           = 15,
        Reset,          // Linux-only
    };

    // TODO: Background colors
}

namespace jc::common {
    // LogLevel //
    enum class LogLevel : uint8_t {
        Dev, // Forces everything to be printed and prefix message with '[DEV]'
        Debug,
        Info,
        Warn,
        Error,
    };

    enum class TitleKind {
        Line,
        Block,
    };

    // TODO!: map for config with collection of allowed args and constexpr check
    struct LoggerConfig {
        LogLevel level;
        bool printOwner{false};
        bool printLevel{true};
        bool colorize{true};
    };

    template<class T>
    inline std::ostream & operator<<(std::ostream & os, const std::vector<T> & vec);

    template<class K, class V>
    inline std::ostream & operator<<(std::ostream & os, const std::map<K, V> & map);

    inline std::ostream & operator<<(std::ostream & os, Color color);

    class Logger {
    public:
        explicit Logger(std::string owner, const LoggerConfig & config)
                : owner(std::move(owner)), config(Config::getInstance().) {}

        virtual ~Logger() = default;

        template<class ...Args>
        const Logger & debug(Args && ...args) const;

        template<class ...Args>
        const Logger & info(Args && ...args) const;

        template<class ...Args>
        const Logger & warn(Args && ...args) const;

        template<class ...Args>
        const Logger & error(Args && ...args) const;

        template<class ...Args>
        const Logger & dev(Args && ...args) const;

        template<class Arg, class ...Args>
        const Logger & raw(Arg && first, Args && ...other) const;

        template<class Arg, class ...Args>
        void colorized(Color color, Arg && first, Args && ...other);

        /// Prints text as title, used only for one-line messages which are not long
        template<class Arg, class ...Args>
        static void printTitleDev(Arg && first, Args && ...other);

        static inline void nl() {
            std::cout << std::endl;
        }

        template<class Arg, class ...Args>
        static void print(Arg && first, Args && ...other);

        template<class Arg, class ...Args>
        static std::string format(Arg && first, Args && ...other);

        LoggerConfig & getConfig() {
            return config;
        }

        // DEBUG //
        template<class Arg, class ...Args>
        static void devPanic(Arg && first, Args && ...other);

        static void notImplemented(const std::string & what) {
            Logger::devPanic("Not implemented error: `" + what + "`");
        }

        template<class Arg, class ...Args>
        static void devDebug(Arg && first, Args && ...other);

    private:
        std::string owner;
        LoggerConfig config;

        const static uint8_t wrapLen{120};

        template<class Arg, class ...Args>
        const Logger & log(LogLevel level, Arg && first, Args && ...other) const;

    public:
        static const std::map<Color, std::string> unixColors;

    private:
        static const std::map<LogLevel, std::string> levelNames;
        static const std::map<LogLevel, Color> levelColors;
    };

#include "common/Logger.inl"

}

#endif // JACY_COMMON_LOGGER_H
