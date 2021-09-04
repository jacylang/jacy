#ifndef JACY_COMMON_CONFIG_H
#define JACY_COMMON_CONFIG_H

#include <set>

#include "cli/CLICommand.h"
#include "log/Logger.h"

namespace jc::common {

    // Compilation config
    class Config {
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

        void applyCLIArgs(const cli::PassedCommand & args);

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
            AstNames,
            AstNodeMap,
            Ribs,
            Resolutions,
            Definitions,

            All,
        };

        static FlagValueMap<PrintKind> printKinds;

        // Note: Order matters (!), we compare discriminants
        enum class CompileDepth : uint8_t {
            Parser,
            NameResolution,
            Lowering,
            Full,
        };

        static FlagValueMap<CompileDepth> compileDepthKinds;

        // Note: Order matters
        enum class BenchmarkKind : uint8_t {
            Verbose, /// Each process, e.g. printing some debug info. Used to check if some debug process runs too long
            SubStage, /// E.g. Lexing at Parsing stage
            Stage, /// The whole one stage, e.g. full Parsing stage
            Final,
        };

        static FlagValueMap<BenchmarkKind> benchmarkKinds;

        static FlagValueMap<log::LogLevel> logLevelKinds;

        enum class ParserExtraDebug : uint8_t {
            No,
            Entries,
            All,
        };

        static FlagValueMap<ParserExtraDebug> parserExtraDebugKinds;

        // Checkers //
    public:
        // Key-value args //
        bool checkMode(Mode mode) const;
        bool checkPrint(PrintKind printKind) const;
        bool checkBenchmark(BenchmarkKind benchmark) const;
        bool checkCompileDepth(CompileDepth compileDepth) const;
        bool checkLogLevel(log::LogLevel logLevel, const std::string & owner = GLOBAL_LOG_LEVEL_NAME) const;
        bool checkParserExtraDebug(ParserExtraDebug parserExtraDebug) const;

        // Bool args //
        bool checkDev() const;

    public:
        log::LogLevel getLogLevel(const std::string & owner = GLOBAL_LOG_LEVEL_NAME) const;
        const std::string & getRootFile() const;

    private:
        std::string rootFile;

        // Key-value args //
        Mode mode{Mode::Source};
        std::set<PrintKind> print{};
        BenchmarkKind benchmark{BenchmarkKind::Final};
        CompileDepth compileDepth{CompileDepth::Full};
        ParserExtraDebug parserExtraDebug{ParserExtraDebug::No};

        constexpr const static auto GLOBAL_LOG_LEVEL_NAME = "global";
        constexpr static auto DEFAULT_LOG_LEVEL = log::LogLevel::Info;
        const static std::vector<std::string> loggerOwners;

        // Pairs of `owner - log::LogLevel`
        std::map<std::string, log::LogLevel> loggerLevels{{GLOBAL_LOG_LEVEL_NAME, DEFAULT_LOG_LEVEL}};

        // Bool args //
        bool dev{false};

        // Debug //
    public:
        std::unordered_map<std::string, std::vector<std::string>> getOptionsMap() const;
    };
}

#endif // JACY_COMMON_CONFIG_H
