#include "config/Configer.h"

namespace jc::config {
    void Configer::applyCLIArgs(const cli::PassedCommand & args) {
        rootFile = args.getEntryFile().unwrap();

        // Apply bool args //
        dev = args.checkBoolFlag("dev");

        // Apply key-value args //

        // `print`
        args.getFlagValues("print").then([&](const auto & values) {
            for (const auto & val : values) {
                print.insert(printKinds.at(val));
            }
        });

        // `compile-depth`
        args.getFlagSingleValue("compile-depth").then([&](const auto & value) {
            compileDepth = compileDepthKinds.at(value);
        });

        // `benchmark`
        args.getFlagSingleValue("benchmark").then([&](const auto & value) {
            benchmark = benchmarkKinds.at(value);
        });

        // `parser-extra-debug`
        args.getFlagSingleValue("parser-extra-debug").then([&](const auto & value) {
            parserExtraDebug = parserExtraDebugKinds.at(value);
        });

        // `log-level`
        args.getFlagSingleValue("log-level").then([&](const auto & value) {
            loggerLevels[GLOBAL_LOG_LEVEL_NAME] = loggerLevels.at(value);
        }).otherwise([&]() {
            loggerLevels[GLOBAL_LOG_LEVEL_NAME] = dev ? log::LogLevel::Dev : DEFAULT_LOG_LEVEL;
        });

        // `*-log-level`
        for (const auto & owner : loggerOwners) {
            const auto & argName = owner + "-log-level";
            args.getFlagSingleValue(argName).then([&](const auto & value) {
                loggerLevels[owner] = logLevelKinds.at(value);
            }).otherwise([&]() {
                loggerLevels[owner] = loggerLevels.at(GLOBAL_LOG_LEVEL_NAME);
            });
        }
    }
}
