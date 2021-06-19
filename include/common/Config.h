#ifndef JACY_COMMON_CONFIG_H
#define JACY_COMMON_CONFIG_H

#include <set>

#include "cli/Args.h"

namespace jc::common {
    template<class T>
    using key_value_arg_map = const std::map<const std::string, const T>;

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

        // Key-value args variants //
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
            ModTree,
            Names,
            Ribs,

            All,
        };

        static key_value_arg_map<PrintKind> printKinds;

        // Note: Order matters (!), we compare discriminants
        enum class CompileDepth : uint8_t {
            Parser,
            NameResolution,
            Full,
        };

        static key_value_arg_map<CompileDepth> compileDepthKinds;

        enum class Benchmark {
            Final,
            EachStage,
        };

        static key_value_arg_map<Benchmark> benchmarkKinds;

        // General for `Config` and `Logger`
        enum class LogLevel : uint8_t {
            Dev, // Forces all logs to be printed and allows special logs for debug with '[DEV]' prefix
            Debug,
            Info,
            Warn,
            Error,

            Unknown,
        };

        static key_value_arg_map<LogLevel> logLevelKinds;

        enum class ParserExtraDebug : uint8_t {
            No,
            Entries,
            All,
        };

        static key_value_arg_map<ParserExtraDebug> parserExtraDebugKinds;

        // Checkers //
    public:
        // Key-value args //
        bool checkMode(Mode mode) const;
        bool checkPrint(PrintKind printKind) const;
        bool checkBenchmark(Benchmark benchmark) const;
        bool checkCompileDepth(CompileDepth compileDepth) const;
        bool checkLogLevel(LogLevel logLevel, const std::string & owner = GLOBAL_LOG_LEVEL_NAME) const;
        bool checkParserExtraDebug(ParserExtraDebug parserExtraDebug) const;

        // Bool args //
        bool checkDev() const;

    public:
        LogLevel getLogLevel(const std::string & owner = GLOBAL_LOG_LEVEL_NAME) const;
        const std::string & getRootFile() const;

    private:
        std::string rootFile;

        // Key-value args //
        Mode mode{Mode::Source};
        std::set<PrintKind> print{};
        Benchmark benchmark{Benchmark::Final};
        CompileDepth compileDepth{CompileDepth::Full};
        ParserExtraDebug parserExtraDebug{ParserExtraDebug::No};

        constexpr const static auto GLOBAL_LOG_LEVEL_NAME = "global";
        constexpr static auto DEFAULT_LOG_LEVEL = LogLevel::Info;
        const static std::vector<std::string> loggerOwners;

        // Pairs of `owner - LogLevel`
        std::unordered_map<std::string, LogLevel> loggerLevels;

        // Bool args //
        bool dev{false};

        // Debug //
    public:
        std::unordered_map<std::string, std::vector<std::string>> getOptionsMap() const;
    };
}

#endif // JACY_COMMON_CONFIG_H
