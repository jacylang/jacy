#include "config/Config.h"

#include <iostream>

namespace jc::config {
    Config::Config() {
        for (const auto & owner : loggerOwners) {
            devLogObjects.emplace(owner, false);
        }

        const auto & cliConfig = cli::getConfig();

        if (cliConfig.has("dev-print-stages")) {
            const auto & _devPrintStages = cliConfig.objAt("dev-print-stages");
            for (const auto & stage : devStagesKinds) {
                const auto & configStage = _devPrintStages.find(stage.first);
                if (configStage == _devPrintStages.end()) {
                    continue;
                }
                const auto & stagePrinters = configStage->second.getArr();
                for (const auto & printer : stagePrinters) {
                    devPrintStages[devPrintKinds.at(printer.getStr())].emplace(stage.second);
                }
            }

            const auto & anyStagePrinters = _devPrintStages.find("$any");
            if (anyStagePrinters != _devPrintStages.end()) {
                for (const auto & printer : anyStagePrinters->second.getArr()) {
                    for (const auto & stage : devStagesKinds) {
                        devPrintStages[devPrintKinds.at(printer.getStr())].emplace(stage.second);
                    }
                }
            }
        }
    }

    Config::FlagValueMap<Config::DevPrint> Config::devPrintKinds = {
        {"suggestions",  Config::DevPrint::Suggestions},
        {"summary",      Config::DevPrint::Summary},
        {"dir-tree",     Config::DevPrint::DirTree},
        {"source",       Config::DevPrint::Source},
        {"tokens",       Config::DevPrint::Tokens},
        {"ast",          Config::DevPrint::Ast},
        {"ast-node-map", Config::DevPrint::AstNodeMap},
        {"ast-names",    Config::DevPrint::AstNames},
        {"mod-tree",     Config::DevPrint::ModTree},
        {"ribs",         Config::DevPrint::Ribs},
        {"definitions",  Config::DevPrint::Definitions},
        {"resolutions",  Config::DevPrint::Resolutions},
        {"all",          Config::DevPrint::All},
    };

    const std::map<std::string, Config::DevStage> Config::devStagesKinds = {
        {"lexer", Config::DevStage::Lexer},
        {"parser", Config::DevStage::Parser},
        {"name-res", Config::DevStage::NameRes},
        {"lowering", Config::DevStage::Lowering},
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

    const std::set<std::string> Config::loggerOwners = {
        "lexer",
        "parser",
        "mod-tree-builder",
        "importer",
        "name-resolver",
        "lowering",
    };

    /////////
    // API //
    /////////

    // Key-value args //
    bool Config::checkMode(Mode mode) const {
        return this->mode == mode;
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

    // Dev mode options
    bool Config::checkDevMode() const {
        return devMode.some() and devMode.unwrap();
    }

    bool Config::checkDevFull() const {
        return devFull.some() and devFull.unwrap();
    }

    bool Config::checkDevLog(const std::string & object) const {
        const auto & found = devLogObjects.find(object);
        if (found == devLogObjects.end()) {
            return checkDevFull();
        }
        return found->second;
    }

    bool Config::checkDevPrint(const DevPrint & printKind) const {
        if (devPrint.find(DevPrint::All) != devPrint.end()) {
            return true;
        }

        const auto & maybeInStage = devPrintStages.find(printKind);
        if (maybeInStage != devPrintStages.end()) {
            for (const auto & stage : maybeInStage->second) {
                if (checkDevStage(stage)) {
                    return true;
                }
            }
        }

        const auto & found = devPrint.find(printKind);
        if (found == devPrint.end()) {
            return checkDevFull();
        }
        return found->second;
    }

    bool Config::checkDevStage(const DevStage & stage) const {
        const auto & found = devStages.find(stage);
        if (found == devStages.end()) {
            return checkDevFull();
        }
        return found->second;
    }

    // API //
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

        // Bool args //
        if (devMode.some()) {
            res["dev"] = {boolOptionStr(devMode)};
        }

        if (devFull.some()) {
            res["dev-full"] = {boolOptionStr(devFull)};
        }

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

        for (const auto & object : devLogObjects) {
            if (object.second) {
                res["dev-log"].emplace_back(object.first);
            }
        }

        for (const auto & stage : devStages) {
            if (not stage.second) {
                continue;
            }

            switch (stage.first) {
                case DevStage::Lexer: {
                    res["dev-stages"].emplace_back("lexer");
                    break;
                }
                case DevStage::Parser: {
                    res["dev-stages"].emplace_back("parser");
                    break;
                }
                case DevStage::NameRes: {
                    res["dev-stages"].emplace_back("name-res");
                    break;
                }
                case DevStage::Lowering: {
                    res["dev-stages"].emplace_back("lowering");
                    break;
                }
            }
        }

        for (const auto & printKind : devPrint) {
            if (not printKind.second) {
                continue;
            }
            switch (printKind.first) {
                case DevPrint::DirTree: {
                    res["print"].emplace_back("dir-tree");
                    break;
                }
                case DevPrint::Summary: {
                    res["print"].emplace_back("summary");
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

        return res;
    }

    std::string Config::boolOptionStr(const Option<bool> & val) const {
        if (val.some()) {
            return val.unwrap() ? "yes" : "no";
        }
        return "[unspecified]";
    }
}
