#include "cli/CLI.h"

namespace jc::cli {
    CLI::CLI() {

    }

    void CLI::applyArgs(int argc, const char ** argv) {
        std::string kvKey; // Key of current key-value argument

        str_vec sourceFiles;
        // Start from 1 to skip bin file path
        for (int i = 1; i < argc; ++i) {
            const std::string arg(argv[i]);

            if (utils::str::startsWith(arg, "--")) {
                // Boolean arg
                config.boolArgs.emplace(arg.substr(2), true);
                kvKey = "";
            } else if (utils::str::startsWith(arg, "-")) {
                kvKey = arg.substr(1);
            } else if (not kvKey.empty()) {
                config.keyValueArgs[kvKey].emplace_back(arg);
            } else {
                bool isSourceFile = false;
                for (const auto & ext : Args::allowedExtensions) {
                    if (utils::str::endsWith(arg, "." + ext)) {
                        sourceFiles.push_back(arg);
                        isSourceFile = true;
                    }
                }
                if (!isSourceFile) {
                    throw CLIError("Unexpected cli argument '" + arg + "'");
                } else {
                    kvKey = "";
                }
            }
        }

        // Check arguments and apply aliases //
        for (auto & arg : config.boolArgs) {
            const auto & alias = Args::aliases.find(arg.first);
            if (not utils::arr::has(Args::allowedBoolArgs, arg.first) and alias == Args::aliases.end()) {
                throw CLIError(common::Logger::format("Unknown argument '", arg.first, "'"));
            }

            if (alias != Args::aliases.end()) {
                utils::map::rename(config.boolArgs, arg.first, alias->second);
            }
        }

        for (const auto & arg : config.keyValueArgs) {
            const auto & alias = Args::aliases.find(arg.first);
            if (not utils::map::has(Args::allowedKeyValueArgs, arg.first) and alias == Args::aliases.end()) {
                throw CLIError(common::Logger::format("Unknown argument '", arg.first, "'"));
            }
            if (arg.second.empty()) {
                throw CLIError(common::Logger::format("Expected value for '", arg.first, "' argument"));
            }

            // Check for allowed parameters for key-value argument if it receives specific list
            if (not utils::arr::has(Args::anyParamKeyValueArgs, arg.first)) {
                for (const auto & argParam : arg.second) {
                    const auto & allowed = Args::allowedKeyValueArgs.at(arg.first).second;
                    if (not utils::arr::has(allowed, argParam)) {
                        throw CLIError(common::Logger::format(
                            "Unknown parameter for argument '", arg.first,
                            "'. Try one of this: ", utils::arr::join(allowed)
                        ));
                    }
                }
            }

            // Check for parameters count
            const auto & allowedCount = Args::allowedKeyValueArgs.at(arg.first).first;
            if (allowedCount != -1 and allowedCount != arg.second.size()) {
                throw CLIError(common::Logger::format(
                    "Expected", allowedCount, "count of parameters for argument", arg.first + ",",
                    "got", arg.second.size(), " parameters: ", utils::arr::join(arg.second, " ")
                ));
            }
            if (alias != Args::aliases.end()) {
                utils::map::rename(config.keyValueArgs, arg.first, alias->second);
            }
        }

        // FIXME: TODO: REMOVE DEFAULTS
        config.boolArgs = utils::map::merge(Args::defaultBoolArgs, config.boolArgs);
        config.keyValueArgs = utils::map::merge(Args::defaultKeyValueArgs, config.keyValueArgs);

        // Check for dependencies //

        // Note: Use vector to output multiple arg-dependency errors
        std::vector<std::string> errorDeps;
        for (const auto & arg : Args::argsDependencies) {
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
                const auto & deps = Args::argsDependencies.at(errorDep);
                errorMsg += utils::arr::join(deps, ", ", {}, {"'", "'"})
                    + " argument" + (deps.size() > 1 ? "s" : "")
                    + " required to be specified for argument '" + errorDep + "'\n";
            }
            throw CLIError(errorMsg);
        }

        if (sourceFiles.size() > 1) {
            throw CLIError("Multiple root files not allowed");
        } else if (sourceFiles.empty()) {
            throw CLIError("REPL Mode is not implemented, please, specify root file");
        }

        config.rootFile = sourceFiles.at(0);

        // Note: Log arguments before dependency check to make it easier for user to find mistake
        {
            // Debug //
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
                "Run jacy " + config.rootFile + " ",
                join(keys(config.boolArgs), " ", {}, {"--"}),
                keyValueArgsStr);
        }

        log.dev("CLI Arguments:\n",
                  "\tBoolean arguments:", config.boolArgs,
                  "\n\tKey-value arguments:", config.keyValueArgs,
                  "\n\tRoot file:", config.rootFile).nl();
    }
}
