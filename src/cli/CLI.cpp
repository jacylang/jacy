#include "cli/CLI.h"

namespace jc::cli {
    CLI::CLI() {

    }

    str_vec CLI::prepareArgs(int argc, const char ** argv) {
        str_vec args;
        // Start from 1 to skip bin file path
        for (int i = 1; i < argc; i++) {
            const std::string arg(argv[i]);
            for (const auto & part : utils::str::splitKeep(arg, Args::delimiters)) {
                args.emplace_back(part);
            }
        }
        return args;
    }

    void CLI::applyArgs(int argc, const char ** argv) {
        const auto & args = prepareArgs(argc, argv);

        str_vec unexpectedArgs;
        str_vec sourceFiles;
        for (size_t argIndex = 0; argIndex < args.size(); argIndex++) {
            const auto & arg = args.at(argIndex);
            if (utils::str::startsWith(arg, "--")) {
                // Boolean arg
                config.boolArgs.emplace(arg.substr(2), true);
            } else if (utils::str::startsWith(arg, "-")) {
                const auto & kvArg = arg.substr(1);
                if (argIndex >= args.size() - 1 or args.at(argIndex + 1) != "=") {
                    throw CLIError("Expected `=` after key-value argument '" + arg + "'");
                }
                argIndex++; // Skip `=`
                str_vec params;
                for (size_t paramIndex = argIndex; paramIndex < args.size(); paramIndex++) {
                    const auto & param = args.at(paramIndex);
                    params.emplace_back(param);
                    if (paramIndex < args.size() - 1 and args.at(paramIndex + 1) != ",") {
                        break;
                    }
                }
                if (params.empty()) {
                    throw CLIError("Expected parameter after `=` for key-value argument '" + arg + "'");
                }
                config.keyValueArgs[kvArg] = params;
            } else {
                bool isSourceFile = false;
                for (const auto & ext : Args::allowedExtensions) {
                    if (utils::str::endsWith(arg, "." + ext)) {
                        sourceFiles.push_back(arg);
                        isSourceFile = true;
                    }
                }
                if (!isSourceFile) {
                    unexpectedArgs.emplace_back(arg);
                }
            }
        }

        if (not unexpectedArgs.empty()) {
            throw CLIError("Unexpected cli argument '" + common::Logger::format(unexpectedArgs) + "'");
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
                            "Unknown parameter '"+ argParam +"' for argument '"+ arg.first +"'.",
                            "Try one of this:", utils::arr::join(allowed)
                        ));
                    }
                }
            }

            // Check for parameters count
            const auto & allowedCount = Args::allowedKeyValueArgs.at(arg.first).first;
            if (allowedCount and allowedCount.unwrap() != arg.second.size()) {
                throw CLIError(common::Logger::format(
                    "Expected", allowedCount.unwrap(), "count of parameters for argument", arg.first + ",",
                    "got", arg.second.size(), " parameters: ", utils::arr::join(arg.second, " ")
                ));
            }
            if (alias != Args::aliases.end()) {
                utils::map::rename(config.keyValueArgs, arg.first, alias->second);
            }
        }

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
