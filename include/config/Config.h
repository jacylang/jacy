#ifndef JACY_CONFIG_CONFIG_H
#define JACY_CONFIG_CONFIG_H

#include <map>
#include <string>
#include <set>
#include <vector>
#include <unordered_map>

namespace jc::config {
    class Configer;

    // Compilation config
    class Config {
        friend Configer;

        template<class T>
        using FlagValueMap = const std::map<const std::string, const T>;
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

        // `dev` //
    public:
        bool dev{false};

        // Mode (unused) //
    public:
        enum class Mode {
            Repl,
            Source,
        };

    private:
        Mode mode{Mode::Source};

        // `print`
    public:
        enum class PrintKind {
            DirTree,
            Ast,
            Tokens,
            Suggestions,
            Source,
            ModTree,
            AstNames,
            AstNodeMap,
            Ribs,
            Resolutions,
            Definitions,

            All,
        };

    private:
        static FlagValueMap<PrintKind> printKinds;
        std::set<PrintKind> print{};

        // `compile-depth`
    public:
        // Note: Order matters (!), we compare discriminants
        enum class CompileDepth : uint8_t {
            Parser,
            NameResolution,
            Lowering,
            Full,
        };

    private:
        static FlagValueMap<CompileDepth> compileDepthKinds;
        CompileDepth compileDepth{CompileDepth::Full};

        // `benchmark`
    public:
        // Note: Order matters
        enum class BenchmarkKind : uint8_t {
            Verbose, /// Each process, e.g. printing some debug info. Used to check if some debug process runs too long
            SubStage, /// E.g. Lexing at Parsing stage
            Stage, /// The whole one stage, e.g. full Parsing stage
            Final,
        };

    private:
        static FlagValueMap<BenchmarkKind> benchmarkKinds;
        BenchmarkKind benchmark{BenchmarkKind::Final};

        // `log-level`, `*-log-level`
    public:
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

    private:
        static FlagValueMap<LogLevel> logLevelKinds;

        constexpr const static auto GLOBAL_LOG_LEVEL_NAME = "global";
        constexpr static auto DEFAULT_LOG_LEVEL = LogLevel::Info;
        const static std::vector<std::string> loggerOwners;

        // Pairs of `owner - LogLevel`
        std::map<std::string, LogLevel> loggerLevels{{GLOBAL_LOG_LEVEL_NAME, DEFAULT_LOG_LEVEL}};

        // `parser-extra-debug` //
    public:
        enum class ParserExtraDebug : uint8_t {
            No,
            Entries,
            All,
        };

    private:
        static FlagValueMap<ParserExtraDebug> parserExtraDebugKinds;
        ParserExtraDebug parserExtraDebug{ParserExtraDebug::No};

        // API //
    public:
        // Bool args //
        bool checkDev() const;

        // Key-value args //
        bool checkMode(Mode mode) const;
        bool checkPrint(PrintKind printKind) const;
        bool checkBenchmark(BenchmarkKind benchmark) const;
        bool checkCompileDepth(CompileDepth compileDepth) const;
        bool checkLogLevel(LogLevel logLevel, const std::string & owner = GLOBAL_LOG_LEVEL_NAME) const;
        bool checkParserExtraDebug(ParserExtraDebug parserExtraDebug) const;

        LogLevel getLogLevel(const std::string & owner = GLOBAL_LOG_LEVEL_NAME) const;
        const std::string & getRootFile() const;

    private:
        std::string rootFile;

        // Debug //
    public:
        std::unordered_map<std::string, std::vector<std::string>> getOptionsMap() const;
    };
}

#endif // JACY_CONFIG_CONFIG_H
