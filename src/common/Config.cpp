#include "common/Config.h"

#include <iostream>

namespace jc::common {
    Config::Config() = default;

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
                } else if (val == "names") {
                    print.insert(PrintKind::Names);
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

        const auto & maybeLogLevel = cliConfig.getSingleValue("log-level");
        if (maybeLogLevel) {
            const auto & ll = maybeLogLevel.unwrap();
            if (ll == "dev") {
                loggerLevels[GLOBAL_LOG_LEVEL_NAME] = LogLevel::Dev;
            } else if (ll == "debug") {
                loggerLevels[GLOBAL_LOG_LEVEL_NAME] = LogLevel::Debug;
            } else if (ll == "info") {
                loggerLevels[GLOBAL_LOG_LEVEL_NAME] = LogLevel::Info;
            } else if (ll == "warn") {
                loggerLevels[GLOBAL_LOG_LEVEL_NAME] = LogLevel::Warn;
            } else if (ll == "error") {
                loggerLevels[GLOBAL_LOG_LEVEL_NAME] = LogLevel::Error;
            } else {
                throw std::logic_error("[Config] Unhandled value for `log-level` cli argument");
            }
        }

        // Apply bool args //
        dev = cliConfig.is("dev");

        if (dev and not maybeLogLevel) {
            // If no `log-level` argument applied and we are in the dev mode, we set it to `Dev`
            loggerLevels[GLOBAL_LOG_LEVEL_NAME] = LogLevel::Dev;
        }
    }

    // Checkers //
    bool Config::checkMode(Mode mode) const {
        return this->mode == mode;
    }

    bool Config::checkPrint(PrintKind printKind) const {
        return print.find(PrintKind::All) != print.end() or print.find(printKind) != print.end();
    }

    bool Config::checkBenchmark(Benchmark benchmark) const {
        return this->benchmark == benchmark;
    }

    bool Config::checkDev() const {
        return dev;
    }

    bool Config::checkCompileDepth(CompileDepth compileDepth) const {
        return static_cast<uint8_t>(this->compileDepth) <= static_cast<uint8_t>(compileDepth);
    }

    bool Config::checkLogLevel(LogLevel logLevel, const std::string & owner) const {
        return static_cast<uint8_t>(this->loggerLevels.at(owner)) <= static_cast<uint8_t>(logLevel);
    }

    Config::LogLevel Config::getGlobalLogLevel() const {
        return loggerLevels.at(GLOBAL_LOG_LEVEL_NAME);
    }

    const std::string & Config::getRootFile() const {
        return rootFile;
    }

    // Debug //
    std::map<std::string, std::vector<std::string>> Config::getOptionsMap() const {
        std::map<std::string, std::vector<std::string>> res;

        res["mode"] = {};
        switch (mode) {
            case Mode::Repl: res["mode"].emplace_back("REPL"); break;
            case Mode::Source: res["mode"].emplace_back("Source"); break;
        }

        res["print"] = {};
        if (print.empty()) {
            res["print"].emplace_back("[none]");
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

        res["compile-depth"] = {};
        switch (compileDepth) {
            case CompileDepth::Parser: res["compile-depth"].emplace_back("parser"); break;
            case CompileDepth::NameResolution: res["compile-depth"].emplace_back("name-resolution"); break;
            case CompileDepth::Full: res["compile-depth"].emplace_back("full"); break;
        }

        res["benchmark"] = {};
        switch (benchmark) {
            case Benchmark::Final: res["benchmark"].emplace_back("final"); break;
            case Benchmark::EachStage: res["benchmark"].emplace_back("each-stage"); break;
        }

        const auto addLogLevel = [&](const std::string & owner) {
            const auto & fieldName = owner == GLOBAL_LOG_LEVEL_NAME ? owner : owner + "-log-level";
            switch (loggerLevels.at(owner)) {
                case LogLevel::Dev: res[fieldName].emplace_back("dev"); break;
                case LogLevel::Debug: res[fieldName].emplace_back("debug"); break;
                case LogLevel::Info: res[fieldName].emplace_back("info"); break;
                case LogLevel::Warn: res[fieldName].emplace_back("warn"); break;
                case LogLevel::Error: res[fieldName].emplace_back("error"); break;
            }
        };

        addLogLevel(GLOBAL_LOG_LEVEL_NAME);
        addLogLevel("lexer");
        addLogLevel("parser");
        addLogLevel("name-resolver");

        return res;
    }
}
