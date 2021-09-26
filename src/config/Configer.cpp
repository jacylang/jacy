#include "config/Configer.h"

namespace jc::config {
    void Configer::applyCLIArgs(const cli::PassedCommand & args) {
        auto & config = Config::getInstance();

        config.rootFile = args.getEntryFile().unwrap();

        // Apply bool args //
        config.devMode = args.checkBoolFlag("dev");
        config.devFull = args.checkBoolFlag("dev-full");

        // Apply key-value args //

        // `dev-print`
        args.getFlagValues("dev-print").then([&](const auto & values) {
            for (const auto & val : values) {
                config.devPrint.emplace(config.devPrintKinds.at(val), true);
            }
        });

        // `dev-log`
        args.getFlagValues("dev-log").then([&](const auto & values) {
            for (const auto & val : values) {
                config.devLogObjects.emplace(val, true);
            }
        });

        // `dev-stages`
        args.getFlagValues("dev-stages").then([&](const auto & values) {
            for (const auto & val : values) {
                config.devStages.emplace(config.devStagesKinds.at(val), true);
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
            config.loggerLevels[config.GLOBAL_LOG_LEVEL_NAME] = config.DEFAULT_LOG_LEVEL;
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
