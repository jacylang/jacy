#ifndef JACY_COMMON_LOGGER_H
#define JACY_COMMON_LOGGER_H

#include "log/utils.h"
#include "common/Error.h"
#include "common/Config.h"
#include "utils/str.h"

namespace jc::log {
    using common::Config;

    // TODO!: map for config with collection of allowed args and constexpr check
    struct LoggerConfig {
        Config::LogLevel level{Config::LogLevel::Unknown};
        bool printOwner{true};
        bool printLevel{true};
        bool colorize{true};
    };

    class Logger {
    public:
        explicit Logger(const std::string & owner) : owner(std::move(owner)) {
            config.level = Config::getInstance().getLogLevel(owner);
        }

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

        template<class ...Args>
        const Logger & raw(Args && ...other) const;

        // Spinners //
    public:
        static constexpr const char * = "⣾⣽⣻⢿⡿⣟⣯⣷";

        static void indicateProcessStart(const std::string & msg) const {
            print("")
        }

        static void indicateProcessEnd() const;

    public:
        template<class ...Args>
        static void colorized(Color color, Args && ...args);

        /// Prints text as title, used only for one-line messages which are not long
        template<class ...Args>
        static void printTitleDev(Args && ...args);

        static inline void nl() {
            std::cout << std::endl;
        }

        template<class ...Args>
        static void print(Args && ...args);

        template<class ...Args>
        static std::string format(Args && ...args);

        LoggerConfig & getConfig() {
            return config;
        }

        // DEV //
        template<class ...Args>
        static void devPanic(Args && ...args);

        static void notImplemented(const std::string & what) {
            Logger::devPanic("Not implemented error: `" + what + "`");
        }

        template<class ...Args>
        static void devDebug(Args && ...args);

        static constexpr uint8_t wrapLen{120};

    private:
        std::string owner;
        LoggerConfig config;

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
        static const std::map<Config::LogLevel, std::string> levelNames;
        static const std::map<Config::LogLevel, Color> levelColors;
    };

    #include "Logger.inl"
}

#endif // JACY_COMMON_LOGGER_H
