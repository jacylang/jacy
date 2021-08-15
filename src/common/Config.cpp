#include "common/Config.h"

#include <iostream>

namespace jc::common {
    Config::Config() = default;

    key_value_arg_map<Config::PrintKind> Config::printKinds = {
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

    key_value_arg_map<Config::CompileDepth> Config::compileDepthKinds = {
        {"parser",          Config::CompileDepth::Parser},
        {"name-resolution", Config::CompileDepth::NameResolution},
        {"lowering",        Config::CompileDepth::Lowering},
    };

    key_value_arg_map<Config::BenchmarkKind> Config::benchmarkKinds = {
        {"verbose",        Config::BenchmarkKind::Verbose},
        {"each-sub-stage", Config::BenchmarkKind::SubStage},
        {"each-stage",     Config::BenchmarkKind::Stage},
        {"final",          Config::BenchmarkKind::Final},
    };

    key_value_arg_map<Config::LogLevel> Config::logLevelKinds = {
        {"dev",   Config::LogLevel::Dev},
        {"debug", Config::LogLevel::Debug},
        {"info",  Config::LogLevel::Info},
        {"warn",  Config::LogLevel::Warn},
        {"error", Config::LogLevel::Error},
    };

    key_value_arg_map<Config::ParserExtraDebug> Config::parserExtraDebugKinds = {
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

    void Config::applyCliCommand(const cli::PassedCommand & command) {
        rootFile = cliConfig.getRootFile();

        // Apply bool args //
        dev = cliConfig.is("dev");

        // Apply key-value args //

        // `print`
        cliConfig.getValues("print").then([&](const auto & values) {
            for (const auto & val : values) {
                print.insert(printKinds.at(val));
            }
        });

        // `compile-depth`
        cliConfig.getSingleValue("compile-depth").then([&](const auto & value) {
            compileDepth = compileDepthKinds.at(value);
        });

        // `benchmark`
        cliConfig.getSingleValue("benchmark").then([&](const auto & value) {
            benchmark = benchmarkKinds.at(value);
        });

        // `parser-extra-debug`
        cliConfig.getSingleValue("parser-extra-debug").then([&](const auto & value) {
            parserExtraDebug = parserExtraDebugKinds.at(value);
        });

        // `log-level`
        cliConfig.getSingleValue("log-level").then([&](const auto & value) {
            loggerLevels[GLOBAL_LOG_LEVEL_NAME] = loggerLevels.at(value);
        }).otherwise([&]() {
            loggerLevels[GLOBAL_LOG_LEVEL_NAME] = dev ? LogLevel::Dev : DEFAULT_LOG_LEVEL;
        });

        // `*-log-level`
        for (const auto & owner : loggerOwners) {
            const auto & argName = owner + "-log-level";
            cliConfig.getSingleValue(argName).then([&](const auto & value) {
                loggerLevels[owner] = logLevelKinds.at(value);
            }).otherwise([&]() {
                loggerLevels[owner] = loggerLevels.at(GLOBAL_LOG_LEVEL_NAME);
            });
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

    bool Config::checkBenchmark(BenchmarkKind benchmark) const {
        return static_cast<uint8_t>(this->benchmark) <= static_cast<uint8_t>(benchmark);
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
                case LogLevel::Dev: {
                    res[fieldName].emplace_back("dev");
                    break;
                }
                case LogLevel::Debug: {
                    res[fieldName].emplace_back("debug");
                    break;
                }
                case LogLevel::Info: {
                    res[fieldName].emplace_back("info");
                    break;
                }
                case LogLevel::Warn: {
                    res[fieldName].emplace_back("warn");
                    break;
                }
                case LogLevel::Error: {
                    res[fieldName].emplace_back("error");
                    break;
                }
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
