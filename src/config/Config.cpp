#include "config/Config.h"

#include <iostream>

namespace jc::config {
    Config::Config() {
        for (const auto & devLogger : devLoggers) {
            devLogStages.emplace(devLogger, false);
        }
    }

    Config::FlagValueMap<Config::DevPrint> Config::devPrintKinds = {
        {"dir-tree",     Config::DevPrint::DirTree},
        {"ast",          Config::DevPrint::Ast},
        {"tokens",       Config::DevPrint::Tokens},
        {"sugg",         Config::DevPrint::Suggestions},
        {"source",       Config::DevPrint::Source},
        {"mod-tree",     Config::DevPrint::ModTree},
        {"ast-names",    Config::DevPrint::AstNames},
        {"ast-node-map", Config::DevPrint::AstNodeMap},
        {"ribs",         Config::DevPrint::Ribs},
        {"resolutions",  Config::DevPrint::Resolutions},
        {"definitions",  Config::DevPrint::Definitions},
        {"all",          Config::DevPrint::All},
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

    Config::FlagValueMap<Config::LogLevel> Config::logLevelKinds = {
        {"debug", Config::LogLevel::Debug},
        {"info",  Config::LogLevel::Info},
        {"warn",  Config::LogLevel::Warn},
        {"error", Config::LogLevel::Error},
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

    bool Config::checkPrint(DevPrint printKind) const {
        return devPrint.find(DevPrint::All) != devPrint.end() or devPrint.find(printKind) != devPrint.end();
    }

    bool Config::checkBenchmark(BenchmarkKind benchmark) const {
        return static_cast<uint8_t>(this->benchmark) <= static_cast<uint8_t>(benchmark);
    }

    bool Config::checkCompileDepth(CompileDepth compileDepth) const {
        return static_cast<uint8_t>(this->compileDepth) <= static_cast<uint8_t>(compileDepth);
    }

    bool Config::checkLogLevel(Config::LogLevel logLevel, const std::string & owner) const {
        if (owner != GLOBAL_LOG_LEVEL_NAME and loggerLevels.find(owner) == loggerLevels.end()) {
            return static_cast<uint8_t>(loggerLevels.at(GLOBAL_LOG_LEVEL_NAME)) <= static_cast<uint8_t>(logLevel);
        }
        return static_cast<uint8_t>(loggerLevels.at(owner)) <= static_cast<uint8_t>(logLevel);
    }

    bool Config::checkParserExtraDebug(ParserExtraDebug parserExtraDebug) const {
        // Note: Check if current `parser-extra-debug` includes or bigger than requested
        return static_cast<uint8_t>(this->parserExtraDebug) >= static_cast<uint8_t>(parserExtraDebug);
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
            case Mode::Repl: {
                res["mode"].emplace_back("REPL");
                break;
            }
            case Mode::Source: {
                res["mode"].emplace_back("Source");
                break;
            }
        }

        for (const auto & printKind : devPrint) {
            switch (printKind) {
                case DevPrint::DirTree: {
                    res["print"].emplace_back("dir-tree");
                    break;
                }
                case DevPrint::Ast: {
                    res["print"].emplace_back("ast");
                    break;
                }
                case DevPrint::Tokens: {
                    res["print"].emplace_back("token");
                    break;
                }
                case DevPrint::Suggestions: {
                    res["print"].emplace_back("suggestions");
                    break;
                }
                case DevPrint::Source: {
                    res["print"].emplace_back("source");
                    break;
                }
                case DevPrint::AstNames: {
                    res["print"].emplace_back("ast-names");
                    break;
                }
                case DevPrint::ModTree: {
                    res["print"].emplace_back("mod-tree");
                    break;
                }
                case DevPrint::AstNodeMap: {
                    res["print"].emplace_back("ast-node-map");
                    break;
                }
                case DevPrint::Ribs: {
                    res["print"].emplace_back("ribs");
                    break;
                }
                case DevPrint::Resolutions: {
                    res["print"].emplace_back("resolutions");
                    break;
                }
                case DevPrint::Definitions: {
                    res["print"].emplace_back("definitions");
                    break;
                }
                case DevPrint::All: {
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
                case Config::LogLevel::Debug: {
                    res[fieldName].emplace_back("debug");
                    break;
                }
                case Config::LogLevel::Info: {
                    res[fieldName].emplace_back("info");
                    break;
                }
                case Config::LogLevel::Warn: {
                    res[fieldName].emplace_back("warn");
                    break;
                }
                case Config::LogLevel::Error: {
                    res[fieldName].emplace_back("error");
                    break;
                }
                case Config::LogLevel::Unknown: {
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
