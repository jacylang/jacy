#ifndef JACY_COMMON_CONFIG_H
#define JACY_COMMON_CONFIG_H

#include <set>

#include "cli/Args.h"

namespace jc::common {
    class Config {
    public:
        Config();
        ~Config() = default;

        // Singleton shit //
        Config(Config const&) = delete;
        void operator=(Config const&) = delete;

        static Config & getInstance() {
            static Config instance;
            return instance;
        }

        void applyCliConfig(const cli::Args & cliConfig);

        // Enums //
        enum class Mode {
            Repl,
            Source,
        };

        enum class PrintKind {
            DirTree,
            Ast,
            Tokens,
            Suggestions,
            Source,
            Names,

            All,
        };

        // Note: Order matters (!), we compare discriminants
        enum class CompileDepth : uint8_t {
            Parser,
            NameResolution,
            Full,
        };

        enum class Benchmark {
            Final,
            EachStage,
        };

        // General for `Config` and `Logger`
        enum class LogLevel : uint8_t {
            Dev, // Forces all logs to be printed and allows special logs for debug with '[DEV]' prefix
            Debug,
            Info,
            Warn,
            Error,

            Unknown,
        };

        // Checkers //
        bool checkMode(Mode mode) const;
        bool checkPrint(PrintKind printKind) const;
        bool checkBenchmark(Benchmark benchmark) const;
        bool checkDev() const;
        bool checkCompileDepth(CompileDepth compileDepth) const;
        bool checkLogLevel(LogLevel logLevel, const std::string & owner = GLOBAL_LOG_LEVEL_NAME) const;
        LogLevel getLogLevel(const std::string & owner = GLOBAL_LOG_LEVEL_NAME) const;
        const std::string & getRootFile() const;

    private:
        std::string rootFile;

        // Key-value args //
        Mode mode{Mode::Source};
        std::set<PrintKind> print{};
        Benchmark benchmark{Benchmark::Final};
        CompileDepth compileDepth{CompileDepth::Full};

        constexpr static auto GLOBAL_LOG_LEVEL_NAME = "global";
        // Pairs of `owner - LogLevel`
        std::map<std::string, LogLevel> loggerLevels{{GLOBAL_LOG_LEVEL_NAME, LogLevel::Info}};

        // Bool args //
        bool dev{false};

        // Debug //
    public:
        std::map<std::string, std::vector<std::string>> getOptionsMap() const;
    };
}

#endif // JACY_COMMON_CONFIG_H
