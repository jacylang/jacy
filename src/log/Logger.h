#ifndef JACY_COMMON_LOGGER_H
#define JACY_COMMON_LOGGER_H

#include "log/data_types.h"
#include "common/Error.h"
#include "config/Config.h"
#include "utils/str.h"

namespace jc::log {
    using namespace config;

    // TODO!: map for config with collection of allowed args and constexpr check
    struct LoggerConfig {
        Config::LogLevel level{Config::LogLevel::Unknown};
        bool printOwner{true};
        bool printLevel{true};
        bool colorize{true};
        bool devLogs{false};
    };

    class Logger {
    public:
        explicit Logger(const std::string & owner);

        ~Logger() = default;

        // Common //
    public:
        LoggerConfig & getConfig() {
            return config;
        }

        // Basic Interface //
    public:
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

        template<class ...Args>
        const Logger & raw(Args && ...other) const;

        // Static Interface //
    public:
        template<class ...Args>
        static void print(Args && ...args);

        static inline void nl() {
            std::cout << std::endl;
        }

        // DEV //
        template<class ...Args>
        static void devDebug(Args && ...args);

        // Pretty printing //
    public:
        template<class ...Args>
        static void colorized(Color color, Args && ...args);

        /// Prints text as title, used only for one-line messages which are not long
        template<class ...Args>
        static void printTitleDev(Args && ...args);

        // Details //
    private:
        template<class ...Rest>
        const Logger & log(Config::LogLevel level, Rest && ...rest) const;

        // Overload's //
    private:
        using os = std::ostream;

        static inline os & out(os & s) {
            return s;
        }

        template<class T>
        static inline os & out(os & s, T && t) {
            return s << t;
        }

        template<class Left, class ...Rest>
        static inline os & out(os & s, Left && left, Rest && ...rest) {
            out(s, std::forward<Left>(left));
            return out(s, std::forward<Rest>(rest)...);
        }

        void printInfo(Config::LogLevel level) const {
            if (config.printLevel) {
                if (config.colorize) {
                    out(std::cout, levelColors.at(level));
                }
                std::cout << levelNames.at(level) << ": ";
                if (config.colorize) {
                    out(std::cout, Color::Reset);
                }
            }

            if (config.printOwner) {
                std::cout << "(" << owner << ") ";
            }
        }

    private:
        std::string owner;
        LoggerConfig config;

        // Constants //
    public:
        static constexpr uint8_t wrapLen{120};

    private:
        static const std::map<Config::LogLevel, std::string> levelNames;
        static const std::map<Config::LogLevel, Color> levelColors;
        static const std::string devLogName;
        static const Color devLogsColor;
    };

    #include "Logger.inl"
}

#endif // JACY_COMMON_LOGGER_H
