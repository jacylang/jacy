#ifndef JACY_COMMON_LOGGER_H
#define JACY_COMMON_LOGGER_H

#include "log/utils.h"
#include "common/Error.h"

namespace jc::log {
    // General for `Config` and `Logger`
    // Note: Order matters
    enum class LogLevel : uint8_t {
        Dev, // Forces all logs to be printed and allows special logs for debug with '[DEV]' prefix
        Debug,
        Info,
        Warn,
        Error,

        Unknown,
    };

    // TODO!: map for config with collection of allowed args and constexpr check
    struct LoggerConfig {
        LogLevel level{LogLevel::Unknown};
        bool printOwner{true};
        bool printLevel{true};
        bool colorize{true};
    };

    class Logger {
    public:
        explicit Logger(const std::string & owner) : owner{std::move(owner)} {
            config.level = Config::getInstance().getLogLevel(owner);
        }

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

        template<class ...Args>
        static std::string fmt(Args && ...args);

        template<class ...Args>
        static void assertLogic(bool expr, Args && ...args);

        // DEV //
        template<class ...Args>
        static void devPanic(Args && ...args);

        static void notImplemented(const std::string & what) {
            Logger::devPanic("Not implemented error: `" + what + "`");
        }

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
        const Logger & log(LogLevel level, Rest && ...rest) const;

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

        void printInfo(LogLevel level) const {
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

        // Advances features //
    private:


    private:
        std::string owner;
        LoggerConfig config;

        // Constants //
    public:
        static constexpr uint8_t wrapLen{120};

    private:
        static const std::map<LogLevel, std::string> levelNames;
        static const std::map<LogLevel, Color> levelColors;
    };

    #include "Logger.inl"
}

#endif // JACY_COMMON_LOGGER_H
