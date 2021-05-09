#include "cli/CLI.h"

namespace jc::cli {
    CLI::CLI() = default;

    void CLI::applyArgs(int argc, const char ** argv) {
        log.dev("Applying cli arguments...");

        bool inKeyValue = false;
        std::string currentKey; // Key of current key-value argument
        std::vector<std::string> values; // Collection of values for current key-value argument

        const auto & collectValues = [&]() {
            config.keyValueArgs.emplace(currentKey, values);
            inKeyValue = false;
        };

        for (int i = 0; i < argc; ++i) {
            const std::string arg(argv[i]);

            if (utils::str::startsWith(arg, "--")) {
                // Boolean arg
                config.boolArgs.emplace(arg.substr(3), true);
                collectValues();
            } else if (utils::str::startsWith(arg, "-")) {
                collectValues();
                currentKey = arg;
                inKeyValue = true;
            } else if (inKeyValue) {
                values.push_back(arg);
            } else {
                for (const auto & ext : Config::allowedExtensions) {
                    if (utils::str::endsWith(arg, "." + ext)) {
                        config.sourceFiles.push_back(arg);
                    }
                }
            }
        }

        // Check arguments and apply aliases //
        for (auto & arg : config.boolArgs) {
            const auto & alias = Config::aliases.find(arg.first);
            if (not utils::arr::has(Config::allowedBoolArgs, arg.first) and alias == Config::aliases.end()) {
                throw CLIError(common::Logger::format("Unknown argument '", arg.first, "'"));
            }

            if (alias != Config::aliases.end()) {
                utils::map::rename(config.boolArgs, arg.first, alias->second);
            }
        }

        for (const auto & arg : config.keyValueArgs) {
            const auto & alias = Config::aliases.find(arg.first);
            if (utils::map::has(Config::allowedKeyValueArgs, arg.first) and alias == Config::aliases.end()) {
                throw CLIError(common::Logger::format("Unknown argument '", arg.first, "'"));
            }
            if (arg.second.empty()) {
                throw CLIError(common::Logger::format("Expected value for '", arg.first, "' argument"));
            }

            // Check for allowed parameters for key-value argument if it receives specific list
            if (not utils::arr::has(Config::anyParamKeyValueArgs, arg.first)) {
                for (const auto & argParam : arg.second) {
                    const auto & allowed = Config::allowedKeyValueArgs.at(arg.first).second;
                    if (not utils::arr::has(allowed, argParam)) {
                        throw CLIError(common::Logger::format(
                            "Unknown parameter for argument '", arg.first,
                            "'. Try one of this: ", utils::arr::join(allowed)
                        ));
                    }
                }
            }

            // Check for parameters count
            const auto & allowedCount = Config::allowedKeyValueArgs.at(arg.first).first;
            if (allowedCount != -1 and allowedCount != arg.second.size()) {
                throw CLIError(common::Logger::format(
                    "Expected", allowedCount, "count of parameters for argument", arg.first + ",",
                    "got", arg.second.size(), " parameters: ", utils::arr::join(arg.second, " ")
                ));
            }
            if (alias != Config::aliases.end()) {
                utils::map::rename(config.keyValueArgs, arg.first, alias->second);
            }
        }

        // Merge with defaults //
        if (dev::DevConfig::dev) {
            config.boolArgs = utils::map::merge(dev::DevConfig::defaultBoolArgs, config.boolArgs);
            config.keyValueArgs = utils::map::merge(dev::DevConfig::defaultKeyValueArgs, config.keyValueArgs);
        } else {
            config.boolArgs = utils::map::merge(Config::defaultBoolArgs, config.boolArgs);
            config.keyValueArgs = utils::map::merge(Config::defaultKeyValueArgs, config.keyValueArgs);
        }

        // TODO: Unify, refactor
//        if (dev::DevConfig::dev) {
//            // DEBUG: Check that default arguments are allowed too //
//            for (auto & arg : config.defaultBoolArgs) {
//                const auto & alias = Config::aliases.find(arg.first);
//                if (not utils::arr::has(Config::allowedBoolArgs, arg.first) and alias == Config::aliases.end()) {
//                    throw CLIError(common::Logger::format("Unknown argument '", arg.first, "'"));
//                }
//
//                if (alias != Config::aliases.end()) {
//                    utils::map::rename(config.boolArgs, arg.first, alias->second);
//                }
//            }
//
//            for (const auto & arg : config.defaultKeyValueArgs) {
//                const auto & alias = Config::aliases.find(arg.first);
//                if (utils::map::has(config.allowedKeyValueArgs, arg.first) and alias == Config::aliases.end()) {
//                    throw CLIError(common::Logger::format("Unknown argument '", arg.first, "'"));
//                }
//                if (arg.second.empty()) {
//                    throw CLIError(common::Logger::format("Expected value for '", arg.first, "' argument"));
//                }
//                // Check for allowed parameters for key-value argument if it receives specific list
//                if (not utils::arr::has(config.anyParamKeyValueArgs, arg.first)) {
//                    for (const auto & argParam : arg.second) {
//                        if (not utils::arr::has(config.allowedKeyValueArgs.at(arg.first).allowed, argParam)) {
//                            throw CLIError(common::Logger::format("Unknown parameter for argument '", arg.first, "'"));
//                        }
//                    }
//                }
//                if (alias != Config::aliases.end()) {
//                    utils::map::rename(config.keyValueArgs, arg.first, alias->second);
//                }
//            }
//        }

        // Note: Log arguments before dependency check to make it easier for user to find mistake
        {
            using utils::arr::join;
            using utils::map::keys;
            std::string keyValueArgsStr;
            for (auto it = config.keyValueArgs.begin(); it != config.keyValueArgs.end(); it++) {
                keyValueArgsStr += "-" + it->first + " " + join(it->second, " ");
                if (it != std::prev(config.keyValueArgs.end())) {
                    keyValueArgsStr += " ";
                }
            }

            log.colorized(
                common::Color::Magenta,
                "Run jacy",
                join(config.sourceFiles, " "),
                join(keys(config.boolArgs), " ", {}, {"--"}),
                keyValueArgsStr);
        }

        // Check for dependencies //

        // Note: Use vector to output multiple arg-dependency errors
        std::vector<std::string> errorDeps;
        for (const auto & arg : Config::argsDependencies) {
            if (config.specified(arg.first)) {
                for (const auto & dep : arg.second) {
                    if (!config.specified(dep)) {
                        errorDeps.push_back(arg.first);
                        break; // We found at least one missing dependency, so break
                    }
                }
            }
        }

        if (not errorDeps.empty()) {
            // Here we check for multiple argument dependency errors and generate an error
            std::string errorMsg;
            for (const auto & errorDep : errorDeps) {
                const auto & deps = Config::argsDependencies.at(errorDep);
                errorMsg += utils::arr::join(deps, ", ", {}, {"'", "'"})
                    + " argument" + (deps.size() > 1 ? "s" : "")
                    + " required to be specified for argument '" + errorDep + "'\n";
            }
            throw CLIError(errorMsg);
        }

        // Set config by args //
        if (config.sourceFiles.empty()) {
            config.mode = Config::Mode::Repl;
        } else {
            config.mode = Config::Mode::Source;
        }

        // DEBUG
        log.debug("CLI Arguments:\n",
                  "\tBoolean arguments", config.boolArgs,
                  "\n\tKey-value arguments: ", config.keyValueArgs,
                  "\n\tSource files: ", config.sourceFiles);
    }
}
