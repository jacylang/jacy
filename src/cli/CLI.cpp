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

    Option<bool> CLI::parseBool(const std::string & val) {
        if (utils::arr::has(Args::boolArgTrueValues, val)) {
            return true;
        }
        if (utils::arr::has(Args::boolArgFalseValues, val)) {
            return false;
        }
        return None;
    }

    void CLI::applyArgs(int argc, const char ** argv) {
        const auto & args = prepareArgs(argc, argv);

        str_vec unexpectedArgs;
        str_vec sourceFiles;
        for (size_t argIndex = 0; argIndex < args.size(); argIndex++) {
            const auto & arg = args.at(argIndex);
            if (utils::str::startsWith(arg, "--")) {
                // Boolean arg
                const auto & argName = arg.substr(2);
                argsStorage.boolArgs.emplace(argName, true);
                if (argIndex < args.size() - 1 and args.at(argIndex + 1) == "=") {
                    argIndex += 2; // Skip arg and `=`
                    if (argIndex == args.size()) {
                        throw CLIError(
                            "Expected boolean parameter after `=` for bool CLI argument '" + arg +
                            "', for example 'yes' or 'no'"
                        );
                    }
                    const auto boolValue = parseBool(utils::str::toLower(args.at(argIndex)));
                    if (boolValue.some()) {
                        argsStorage.boolArgs[argName] = boolValue.unwrap("CLI::applyArgs::boolValue");
                    } else {
                        throw CLIError(
                            "Invalid boolean parameter for bool CLI argument '" + arg +
                            "', expected for example 'yes' or 'no'"
                        );
                    }
                }
            } else if (utils::str::startsWith(arg, "-")) {
                const auto & kvArg = arg.substr(1);
                if (argIndex >= args.size() - 1 or args.at(argIndex + 1) != "=") {
                    throw CLIError("Expected `=` after key-value argument '" + arg + "'");
                }
                argIndex += 2; // Skip argument and `=`
                str_vec params;
                for (size_t paramIndex = argIndex; paramIndex < args.size(); paramIndex++) {
                    const auto & param = args.at(paramIndex);
                    params.emplace_back(param);
                    argIndex = paramIndex;
                    if (paramIndex < args.size() - 1 and args.at(paramIndex + 1) != ",") {
                        break;
                    }
                }
                if (params.empty()) {
                    throw CLIError("Expected parameters after `=` for key-value argument '" + arg + "'");
                }
                argsStorage.keyValueArgs[kvArg] = params;
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

        if (sourceFiles.empty()) {
            throw CLIError("Root file is not specified");
        }

        if (sourceFiles.size() > 1) {
            throw CLIError("Multiple input files are not allowed");
        }

        argsStorage.rootFile = sourceFiles.at(0);

        // Note: Log arguments before error checks to make it easier to find mistakes
        {
            using utils::arr::join;
            using utils::map::keys;
            std::string boolArgsStr;
            for (auto it = argsStorage.boolArgs.begin(); it != argsStorage.boolArgs.end(); it++) {
                boolArgsStr += "--" + it->first + "=" + (it->second ? "1" : "0");
                if (it != std::prev(argsStorage.boolArgs.end())) {
                    boolArgsStr += " ";
                }
            }

            std::string keyValueArgsStr;
            for (auto it = argsStorage.keyValueArgs.begin(); it != argsStorage.keyValueArgs.end(); it++) {
                keyValueArgsStr += "-" + it->first + "=" + join(it->second, ", ");
                if (it != std::prev(argsStorage.keyValueArgs.end())) {
                    keyValueArgsStr += " ";
                }
            }

            log.colorized(
                common::Color::Magenta, "Run jacy " + argsStorage.rootFile, ' ', boolArgsStr, ' ', keyValueArgsStr
            );
        }

        if (not unexpectedArgs.empty()) {
            throw CLIError("Unexpected cli arguments: '" + common::Logger::format(unexpectedArgs) + "'");
        }

        // Check arguments and apply aliases //
        for (auto & arg : argsStorage.boolArgs) {
            const auto & alias = Args::aliases.find(arg.first);
            if (not utils::arr::has(Args::allowedBoolArgs, arg.first) and alias == Args::aliases.end()) {
                throw CLIError("Unknown boolean argument '" + arg.first + "'");
            }

            if (alias != Args::aliases.end()) {
                utils::map::rename(argsStorage.boolArgs, arg.first, alias->second);
            }
        }

        for (const auto & arg : argsStorage.keyValueArgs) {
            const auto & alias = Args::aliases.find(arg.first);
            if (not utils::map::has(Args::allowedKeyValueArgs, arg.first) and alias == Args::aliases.end()) {
                throw CLIError("Unknown key-value argument '" + arg.first + "'");
            }
            if (arg.second.empty()) {
                throw CLIError("Expected value for '" + arg.first + "' argument");
            }

            // Check for allowed parameters for key-value argument if it receives specific list
            if (not utils::arr::has(Args::anyParamKeyValueArgs, arg.first)) {
                for (const auto & argParam : arg.second) {
                    const auto & allowed = Args::allowedKeyValueArgs.at(arg.first).second;
                    if (not utils::arr::has(allowed, argParam)) {
                        throw CLIError(
                            common::Logger::format(
                                "Unknown parameter '" + argParam + "' for argument '" + arg.first + "'. ",
                                "Try one of this: ",
                                utils::arr::join(allowed)
                            )
                        );
                    }
                }
            }

            // Check for parameters count
            const auto & allowedCount = Args::allowedKeyValueArgs.at(arg.first).first;
            if (allowedCount.some() and allowedCount.unwrap("CLI::applyArgs::allowedCount") != arg.second.size()) {
                throw CLIError(
                    common::Logger::format(
                        "Expected ",
                        allowedCount.unwrap(),
                        " count of parameters for argument ",
                        arg.first + ", ",
                        "got ",
                        arg.second.size(),
                        " parameters: ",
                        utils::arr::join(arg.second, " ")
                    )
                );
            }
            if (alias != Args::aliases.end()) {
                utils::map::rename(argsStorage.keyValueArgs, arg.first, alias->second);
            }
        }

        // Check for dependencies //

        // Note: Use vector to output multiple arg-dependency errors
        std::vector<std::string> errorDeps;
        for (const auto & arg : Args::argsDependencies) {
            if (argsStorage.specified(arg.first)) {
                for (const auto & dep : arg.second) {
                    if (!argsStorage.specified(dep)) {
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
                errorMsg += utils::arr::join(deps, ", ", {}, {"'", "'"}) + " argument" + (deps.size() > 1 ? "s" : "") +
                            " required to be enabled for argument '" + errorDep + "'\n";
            }
            throw CLIError(errorMsg);
        }

        log.dev(
            "CLI Arguments:\n",
            "\tBoolean arguments: ",
            argsStorage.boolArgs,
            "\n\tKey-value arguments: ",
            argsStorage.keyValueArgs,
            "\n\tRoot file: ",
            argsStorage.rootFile
        ).nl();
    }
}
