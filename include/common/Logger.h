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

#include "dev/DevConfig.h"
#include "common/Error.h"

namespace jc::common {
    enum class Color {
        Red,
        Green,
        Blue,
        Yellow,
        Magenta,
        Cyan,
    };

    // LogLevel //
    enum class LogLevel : uint8_t {
        Dev, // Forces everything to be printed and prefix message with '[DEV]'
        Verbose, /// @deprecated (use `dev` instead)
        Debug,
        Info,
        Warn,
        Error,
    };

    // TODO!: map for config with collection of allowed args and constexpr check
    struct LoggerConfig {
        LoggerConfig() {
            // NOTE: Force to use `dev` level
            if (dev::DevConfig::dev) {
                level = LogLevel::Dev;
            }
        }

        LogLevel level{LogLevel::Debug};
        bool printOwner{false};
        bool printLevel{true};
        bool colorize{true};
    };

    inline std::ostream & operator<<(std::ostream & os, const std::vector<std::string> & vec) {
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

    template<class V>
    inline std::ostream & operator<<(std::ostream & os, const std::map<std::string, V> & map) {
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

    class Logger {
    public:
        Logger(std::string owner, const LoggerConfig & config) : owner(std::move(owner)), config(config) {}
        virtual ~Logger() = default;

        template<class ...Args>
        void verbose(Args && ...args);

        template<class ...Args>
        void debug(Args && ...args);

        template<class ...Args>
        void info(Args && ...args);

        template<class ...Args>
        void warn(Args && ...args);

        template<class ...Args>
        void error(Args && ...args);

        template<class ...Args>
        void dev(Args && ...args);

        template<class ...Args>
        void devPanic(Args && ...args);

        template<class Arg, class ...Args>
        Logger & raw(Arg && first, Args && ...other);

        template<class Arg, class ...Args>
        void colorized(Color color, Arg && first, Args && ...other);

        void nl() {
            std::cout << std::endl;
        }

        template<class Arg, class ...Args>
        static std::string format(Arg && first, Args && ...other);

        LoggerConfig & getConfig() {
            return config;
        }

    private:
        std::string owner;
        LoggerConfig config;

        template<class Arg, class ...Args>
        void log(LogLevel level, Arg && first, Args && ...other);

        static const std::map<LogLevel, std::string> levelNames;
        static const std::map<LogLevel, Color> levelColors;
        static const std::map<Color, std::string> colors;
        static const std::string ansiReset;
    };

    #include "common/Logger.inl"
}

#endif // JACY_LOGGER_H
