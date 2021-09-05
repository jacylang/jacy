#include "config/Configer.h"

namespace jc::config {
    void Configer::applyCLIArgs(const cli::PassedCommand & args) {
        auto & config = Config::getInstance();

        config.rootFile = args.getEntryFile().unwrap();

        // Apply bool args //
        config.dev = args.checkBoolFlag("dev");

        // Apply key-value args //

        // `print`
        args.getFlagValues("print").then([&](const auto & values) {
            for (const auto & val : values) {
                config.print.insert(config.printKinds.at(val));
            }
        });

        // `compile-depth`
        args.getFlagSingleValue("compile-depth").then([&](const auto & value) {
            config.compileDepth = config.compileDepthKinds.at(value);
        });

        // `benchmark`
        args.getFlagSingleValue("benchmark").then([&](const auto & value) {
            config.benchmark = config.benchmarkKinds.at(value);
        });

        // `parser-extra-debug`
        args.getFlagSingleValue("parser-extra-debug").then([&](const auto & value) {
            config.parserExtraDebug = config.parserExtraDebugKinds.at(value);
        });

        // `log-level`
        args.getFlagSingleValue("log-level").then([&](const auto & value) {
            config.loggerLevels[config.GLOBAL_LOG_LEVEL_NAME] = config.loggerLevels.at(value);
        }).otherwise([&]() {
            config.loggerLevels[config.GLOBAL_LOG_LEVEL_NAME] = config.dev ? log::LogLevel::Dev
                                                                           : config.DEFAULT_LOG_LEVEL;
        });

        // `*-log-level`
        for (const auto & owner : config.loggerOwners) {
            const auto & argName = owner + "-log-level";
            args.getFlagSingleValue(argName).then([&](const auto & value) {
                config.loggerLevels[owner] = config.logLevelKinds.at(value);
            }).otherwise([&]() {
                config.loggerLevels[owner] = config.loggerLevels.at(config.GLOBAL_LOG_LEVEL_NAME);
            });
        }
    }
}
