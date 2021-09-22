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

        // Dev Mode //
    public:

        // Mode (unused) //
    public:
        enum class Mode {
            Repl,
            Source,
        };

    private:
        Mode mode{Mode::Source};

    private:
        bool devMode{false};

        // `--dev-print`
    public:
        enum class DevPrint {
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
        static FlagValueMap<DevPrint> devPrintKinds;
        std::set<DevPrint> devPrint{};

    private:
        std::map<std::string, bool> devLogStages;
        const static std::vector<std::string> devLoggers;

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

        // Options API //
    public:
        // Key-value options //
        bool checkMode(Mode mode) const;
        bool checkPrint(DevPrint printKind) const;
        bool checkBenchmark(BenchmarkKind benchmark) const;
        bool checkCompileDepth(CompileDepth compileDepth) const;
        bool checkLogLevel(LogLevel logLevel, const std::string & owner = GLOBAL_LOG_LEVEL_NAME) const;
        bool checkParserExtraDebug(ParserExtraDebug parserExtraDebug) const;

        // Dev Mode Options
        bool checkDevLog(const std::string & stage);
        bool checkDevPrint(const DevPrint & entity);

        // API //
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
