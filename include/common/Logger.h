#ifndef JACY_LOGGER_H
#define JACY_LOGGER_H

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

namespace jc::common {
    enum class Color {
        Red,
        Green,
        Blue,
        Yellow,
        Magenta,
        Cyan,
        Reset,
    };

    // LogLevel //
    enum class LogLevel : uint8_t {
        Dev, // Forces everything to be printed and prefix message with '[DEV]'
        Debug,
        Info,
        Warn,
        Error,
    };

    // TODO!: map for config with collection of allowed args and constexpr check
    struct LoggerConfig {
        LogLevel level{LogLevel::Debug};
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
        explicit Logger(std::string owner) : owner(std::move(owner)) {}
        virtual ~Logger() = default;

        void init(const LoggerConfig & config) {
            this->config = config;

            // Force Dev level in case of `dev` argument applied
            if (common::Config::getInstance().checkDev()) {
                this->config.level = LogLevel::Dev;
            }
        }

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

        static inline void nl()  {
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

        template<class Arg, class ...Args>
        const Logger & log(LogLevel level, Arg && first, Args && ...other) const;

    public:
        static const std::map<Color, std::string> colors;

    private:
        static const std::map<LogLevel, std::string> levelNames;
        static const std::map<LogLevel, Color> levelColors;
    };

    #include "common/Logger.inl"
}

#endif // JACY_LOGGER_H
