#include "common/Config.h"

#include <iostream>

namespace jc::common {
    Config::Config() = default;

    key_value_arg_map<Config::PrintKind> Config::printKinds = {
        {"dir-tree", Config::PrintKind::DirTree},
        {"ast", Config::PrintKind::Ast},
        {"tokens", Config::PrintKind::Tokens},
        {"sugg", Config::PrintKind::Suggestions},
        {"source", Config::PrintKind::Source},
        {"mod-tree", Config::PrintKind::ModTree},
        {"names", Config::PrintKind::Names},
        {"ribs", Config::PrintKind::Ribs},
        {"all", Config::PrintKind::All},
    };

    key_value_arg_map<Config::CompileDepth> Config::compileDepthKinds = {
        {"parser", Config::CompileDepth::Parser},
        {"name-resolution", Config::CompileDepth::NameResolution},
    };

    key_value_arg_map<Config::Benchmark> Config::benchmarkKinds = {
        {"each-stage", Config::Benchmark::EachStage},
        {"final", Config::Benchmark::Final},
    };

    key_value_arg_map<Config::LogLevel> Config::logLevelKinds = {
        {"dev", Config::LogLevel::Dev},
        {"debug", Config::LogLevel::Debug},
        {"info", Config::LogLevel::Info},
        {"warn", Config::LogLevel::Warn},
        {"error", Config::LogLevel::Error},
    };

    void Config::applyCliConfig(const cli::Args & cliConfig) {
        rootFile = cliConfig.getRootFile();

        // Apply key-value args //

        // `print`
        const auto & printVals = cliConfig.getValues("print");
        if (printVals) {
            for (const auto & val : printVals.unwrap()) {
                if (val == "dir-tree") {
                    print.insert(PrintKind::DirTree);
                } else if (val == "ast") {
                    print.insert(PrintKind::Ast);
                } else if (val == "tokens") {
                    print.insert(PrintKind::Tokens);
                } else if (val == "sugg") {
                    print.insert(PrintKind::Suggestions);
                } else if (val == "source") {
                    print.insert(PrintKind::Source);
                } else if (val == "mod-tree") {
                    print.insert(PrintKind::ModTree);
                } else if (val == "names") {
                    print.insert(PrintKind::Names);
                } else if (val == "ribs") {
                    print.insert(PrintKind::Ribs);
                } else if (val == "all") {
                    print.insert(PrintKind::All);
                } else {
                    throw std::logic_error("[Config] Unhandled value for `print` cli argument");
                }
            }
        }

        // `compile-depth`
        const auto & maybeCompileDepth = cliConfig.getSingleValue("compile-depth");
        if (maybeCompileDepth) {
            const auto & cd = maybeCompileDepth.unwrap();
            if (cd == "parser") {
                compileDepth = CompileDepth::Parser;
            } else if (cd == "name-resolution") {
                compileDepth = CompileDepth::NameResolution;
            } else {
                throw std::logic_error("[Config] Unhandled value for `compile-depth` cli argument");
            }
        }

        // `benchmark`
        const auto & maybeBenchmark = cliConfig.getSingleValue("benchmark");
        if (maybeBenchmark) {
            const auto & bmk = maybeBenchmark.unwrap();
            if (bmk == "final") {
                benchmark = Benchmark::Final;
            } else if (bmk == "each-stage") {
                benchmark = Benchmark::EachStage;
            } else {
                throw std::logic_error("[Config] Unhandled value for `benchmark` cli argument");
            }
        }

        // `parser-extra-debug`
        const auto & maybeParserExtraDebug = cliConfig.getSingleValue("parser-extra-debug");
        if (maybeParserExtraDebug) {
            const auto & ped = maybeParserExtraDebug.unwrap();
            if (ped == "no" or ped == "0") {
                parserExtraDebug = ParserExtraDebug::No;
            } else if (ped == "entries" or ped == "1") {
                parserExtraDebug = ParserExtraDebug::Entries;
            } else if (ped == "all" or ped == "2") {
                parserExtraDebug = ParserExtraDebug::All;
            } else {
                throw std::logic_error("[Config] Unhandled value for `parser-extra-debug` cli argument");
            }
        }

        // `log-level` and `*-log-level`
        bool globalLogLevelAppeared = false;
        for (const auto & owner : std::vector<std::string>{GLOBAL_LOG_LEVEL_NAME, "lexer", "parser", "name-resolver"}) {
            const auto isGlobal = owner == GLOBAL_LOG_LEVEL_NAME;
            const auto & argName = isGlobal ? "log-level" : owner + "-log-level";
            const auto & maybeLogLevel = cliConfig.getSingleValue(argName);
            globalLogLevelAppeared = isGlobal and maybeLogLevel;
            if (maybeLogLevel) {
                const auto & ll = maybeLogLevel.unwrap();
                if (ll == "dev") {
                    loggerLevels[owner] = LogLevel::Dev;
                } else if (ll == "debug") {
                    loggerLevels[owner] = LogLevel::Debug;
                } else if (ll == "info") {
                    loggerLevels[owner] = LogLevel::Info;
                } else if (ll == "warn") {
                    loggerLevels[owner] = LogLevel::Warn;
                } else if (ll == "error") {
                    loggerLevels[owner] = LogLevel::Error;
                } else {
                    throw std::logic_error("[Config] Unhandled value for `log-level` cli argument");
                }
            } else if (not isGlobal) {
                loggerLevels[owner] = DEFAULT_LOG_LEVEL;
            }
        }

        // Apply bool args //
        dev = cliConfig.is("dev");

        if (dev and not globalLogLevelAppeared) {
            // If no `log-level` argument applied and we are in the dev mode, we set it to `Dev`
            loggerLevels[GLOBAL_LOG_LEVEL_NAME] = LogLevel::Dev;
        }
    }

    // Checkers //

    // Key-value args //
    bool Config::checkMode(Mode mode) const {
        return this->mode == mode;
    }

    bool Config::checkPrint(PrintKind printKind) const {
        return print.find(PrintKind::All) != print.end() or print.find(printKind) != print.end();
    }

    bool Config::checkBenchmark(Benchmark benchmark) const {
        return this->benchmark == benchmark;
    }

    bool Config::checkCompileDepth(CompileDepth compileDepth) const {
        return static_cast<uint8_t>(this->compileDepth) <= static_cast<uint8_t>(compileDepth);
    }

    bool Config::checkLogLevel(LogLevel logLevel, const std::string & owner) const {
        if (owner != GLOBAL_LOG_LEVEL_NAME and loggerLevels.find(owner) == loggerLevels.end()) {
            return static_cast<uint8_t>(loggerLevels.at(GLOBAL_LOG_LEVEL_NAME)) <= static_cast<uint8_t>(logLevel);
        }
        return static_cast<uint8_t>(loggerLevels.at(owner)) <= static_cast<uint8_t>(logLevel);
    }

    bool Config::checkParserExtraDebug(ParserExtraDebug parserExtraDebug) const {
        // Note: Check if current `parser-extra-debug` includes or bigger than requested
        return static_cast<uint8_t>(this->parserExtraDebug) >= static_cast<uint8_t>(parserExtraDebug);
    }

    // Bool args //
    bool Config::checkDev() const {
        return dev;
    }

    Config::LogLevel Config::getLogLevel(const std::string & owner) const {
        if (loggerLevels.find(owner) == loggerLevels.end()) {
            return loggerLevels.at(GLOBAL_LOG_LEVEL_NAME);
        }
        return loggerLevels.at(owner);
    }

    const std::string & Config::getRootFile() const {
        return rootFile;
    }

    // Debug //
    std::unordered_map<std::string, std::vector<std::string>> Config::getOptionsMap() const {
        std::unordered_map<std::string, std::vector<std::string>> res;

        // Key-value args //
        switch (mode) {
            case Mode::Repl: res["mode"].emplace_back("REPL"); break;
            case Mode::Source: res["mode"].emplace_back("Source"); break;
        }

        for (const auto & printKind : print) {
            switch (printKind) {
                case PrintKind::DirTree: res["print"].emplace_back("dir-tree"); break;
                case PrintKind::Ast: res["print"].emplace_back("ast"); break;
                case PrintKind::Tokens: res["print"].emplace_back("token"); break;
                case PrintKind::Suggestions: res["print"].emplace_back("suggestions"); break;
                case PrintKind::Source: res["print"].emplace_back("source"); break;
                case PrintKind::Names: res["print"].emplace_back("names"); break;
                case PrintKind::All: res["print"].emplace_back("all"); break;
            }
        }

        switch (compileDepth) {
            case CompileDepth::Parser: res["compile-depth"].emplace_back("parser"); break;
            case CompileDepth::NameResolution: res["compile-depth"].emplace_back("name-resolution"); break;
            case CompileDepth::Full: res["compile-depth"].emplace_back("full"); break;
        }

        switch (benchmark) {
            case Benchmark::Final: res["benchmark"].emplace_back("final"); break;
            case Benchmark::EachStage: res["benchmark"].emplace_back("each-stage"); break;
        }

        const auto addLogLevel = [&](const std::string & owner) {
            const auto & fieldName = owner == GLOBAL_LOG_LEVEL_NAME ? "log-level" : owner + "-log-level";
            switch (loggerLevels.at(owner)) {
                case LogLevel::Dev: res[fieldName].emplace_back("dev"); break;
                case LogLevel::Debug: res[fieldName].emplace_back("debug"); break;
                case LogLevel::Info: res[fieldName].emplace_back("info"); break;
                case LogLevel::Warn: res[fieldName].emplace_back("warn"); break;
                case LogLevel::Error: res[fieldName].emplace_back("error"); break;
                case LogLevel::Unknown: {
                    throw std::logic_error("[Config] `Unknown` log-level found in `Config::getOptionsMap`");
                }
            }
        };

        addLogLevel(GLOBAL_LOG_LEVEL_NAME);
        addLogLevel("lexer");
        addLogLevel("parser");
        addLogLevel("name-resolver");

        // Bool args //
        res["dev"] = {checkDev() ? "yes" : "no"};

        return res;
    }
}
