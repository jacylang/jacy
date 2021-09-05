#include "config/Config.h"

#include <iostream>

namespace jc::config {
    Config::Config() = default;

    Config::FlagValueMap<Config::PrintKind> Config::printKinds = {
        {"dir-tree",     Config::PrintKind::DirTree},
        {"ast",          Config::PrintKind::Ast},
        {"tokens",       Config::PrintKind::Tokens},
        {"sugg",         Config::PrintKind::Suggestions},
        {"source",       Config::PrintKind::Source},
        {"mod-tree",     Config::PrintKind::ModTree},
        {"ast-names",    Config::PrintKind::AstNames},
        {"ast-node-map", Config::PrintKind::AstNodeMap},
        {"ribs",         Config::PrintKind::Ribs},
        {"resolutions",  Config::PrintKind::Resolutions},
        {"definitions",  Config::PrintKind::Definitions},
        {"all",          Config::PrintKind::All},
    };

    Config::FlagValueMap<Config::CompileDepth> Config::compileDepthKinds = {
        {"parser",          Config::CompileDepth::Parser},
        {"name-resolution", Config::CompileDepth::NameResolution},
        {"lowering",        Config::CompileDepth::Lowering},
    };

    Config::FlagValueMap<Config::BenchmarkKind> Config::benchmarkKinds = {
        {"verbose",        Config::BenchmarkKind::Verbose},
        {"each-sub-stage", Config::BenchmarkKind::SubStage},
        {"each-stage",     Config::BenchmarkKind::Stage},
        {"final",          Config::BenchmarkKind::Final},
    };

    Config::FlagValueMap<log::LogLevel> Config::logLevelKinds = {
        {"dev",   log::LogLevel::Dev},
        {"debug", log::LogLevel::Debug},
        {"info",  log::LogLevel::Info},
        {"warn",  log::LogLevel::Warn},
        {"error", log::LogLevel::Error},
    };

    Config::FlagValueMap<Config::ParserExtraDebug> Config::parserExtraDebugKinds = {
        {"no",      Config::ParserExtraDebug::No},
        {"entries", Config::ParserExtraDebug::Entries},
        {"all",     Config::ParserExtraDebug::All},

        {"0",       Config::ParserExtraDebug::No},
        {"1",       Config::ParserExtraDebug::Entries},
        {"2",       Config::ParserExtraDebug::All},
    };

    const std::vector<std::string> Config::loggerOwners = {
        "lexer", "parser", "name-resolver"
    };

    /////////
    // API //
    /////////

    // Key-value args //
    bool Config::checkMode(Mode mode) const {
        return this->mode == mode;
    }

    bool Config::checkPrint(PrintKind printKind) const {
        return print.find(PrintKind::All) != print.end() or print.find(printKind) != print.end();
    }

    bool Config::checkBenchmark(BenchmarkKind benchmark) const {
        return static_cast<uint8_t>(this->benchmark) <= static_cast<uint8_t>(benchmark);
    }

    bool Config::checkCompileDepth(CompileDepth compileDepth) const {
        return static_cast<uint8_t>(this->compileDepth) <= static_cast<uint8_t>(compileDepth);
    }

    bool Config::checkLogLevel(log::LogLevel logLevel, const std::string & owner) const {
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

    log::LogLevel Config::getLogLevel(const std::string & owner) const {
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
            case Mode::Repl: {
                res["mode"].emplace_back("REPL");
                break;
            }
            case Mode::Source: {
                res["mode"].emplace_back("Source");
                break;
            }
        }

        for (const auto & printKind : print) {
            switch (printKind) {
                case PrintKind::DirTree: {
                    res["print"].emplace_back("dir-tree");
                    break;
                }
                case PrintKind::Ast: {
                    res["print"].emplace_back("ast");
                    break;
                }
                case PrintKind::Tokens: {
                    res["print"].emplace_back("token");
                    break;
                }
                case PrintKind::Suggestions: {
                    res["print"].emplace_back("suggestions");
                    break;
                }
                case PrintKind::Source: {
                    res["print"].emplace_back("source");
                    break;
                }
                case PrintKind::AstNames: {
                    res["print"].emplace_back("ast-names");
                    break;
                }
                case PrintKind::ModTree: {
                    res["print"].emplace_back("mod-tree");
                    break;
                }
                case PrintKind::AstNodeMap: {
                    res["print"].emplace_back("ast-node-map");
                    break;
                }
                case PrintKind::Ribs: {
                    res["print"].emplace_back("ribs");
                    break;
                }
                case PrintKind::Resolutions: {
                    res["print"].emplace_back("resolutions");
                    break;
                }
                case PrintKind::Definitions: {
                    res["print"].emplace_back("definitions");
                    break;
                }
                case PrintKind::All: {
                    res["print"].emplace_back("all");
                    break;
                }
            }
        }

        switch (compileDepth) {
            case CompileDepth::Parser: {
                res["compile-depth"].emplace_back("parser");
                break;
            }
            case CompileDepth::NameResolution: {
                res["compile-depth"].emplace_back("name-resolution");
                break;
            }
            case CompileDepth::Lowering: {
                res["compile-depth"].emplace_back("lowering");
                break;
            }
            case CompileDepth::Full: {
                res["compile-depth"].emplace_back("full");
                break;
            }
        }

        switch (benchmark) {
            case BenchmarkKind::Final: {
                res["benchmark"].emplace_back("final");
                break;
            }
            case BenchmarkKind::Stage: {
                res["benchmark"].emplace_back("stage");
                break;
            }
            case BenchmarkKind::SubStage: {
                res["benchmark"].emplace_back("sub-stage");
                break;
            }
            case BenchmarkKind::Verbose: {
                res["benchmark"].emplace_back("verbose");
                break;
            }
        }

        const auto addLogLevel = [&](const std::string & owner) {
            const auto & fieldName = owner == GLOBAL_LOG_LEVEL_NAME ? "log-level" : owner + "-log-level";
            switch (loggerLevels.at(owner)) {
                case log::LogLevel::Dev: {
                    res[fieldName].emplace_back("dev");
                    break;
                }
                case log::LogLevel::Debug: {
                    res[fieldName].emplace_back("debug");
                    break;
                }
                case log::LogLevel::Info: {
                    res[fieldName].emplace_back("info");
                    break;
                }
                case log::LogLevel::Warn: {
                    res[fieldName].emplace_back("warn");
                    break;
                }
                case log::LogLevel::Error: {
                    res[fieldName].emplace_back("error");
                    break;
                }
                case log::LogLevel::Unknown: {
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
