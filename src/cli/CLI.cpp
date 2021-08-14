#include "cli/CLI.h"

namespace jc::cli {
    CLI::CLI() {
        loadConfig();
    }

    str_vec CLI::prepareArgs(int argc, const char ** argv) {
        using namespace utils::str;

        str_vec args;
        // Start from 1 to skip bin file path
        for (int i = 1; i < argc; i++) {
            const std::string arg(argv[i]);
            for (const auto & part : splitKeep(arg, Args::delimiters)) {
                if (part.size() > 2 and part.at(0) == '-' and part.at(1) != '-') {
                    // If arg starts with `-` then it is a one-letter alias, so we can split it.
                    // For example, for `-O0` it will produce `-0`, `0`
                    args.emplace_back(part.substr(0, 2));
                    args.emplace_back(part.substr(2));
                } else {
                    args.emplace_back(part);
                }
            }
        }
        return args;
    }

    Option<bool> CLI::parseBool(const std::string & val) {
        if (utils::arr::has(boolArgTrueValues, val)) {
            return true;
        }
        if (utils::arr::has(boolArgFalseValues, val)) {
            return false;
        }
        return None;
    }

    const Command & CLI::getCommand(const std::string & name) const {
        const auto & found = commands.find(name);
        if (found == commands.end()) {
            throw CLIError("Command '" + name + "' does not exists");
        }
        return found->second;
    }

    void CLI::applyArgs(int argc, const char ** argv) {
        using namespace utils::str;

        const auto & args = prepareArgs(argc, argv);

        bool commandDefaulted = false;
        Option<Command> passedCom{None};
        std::vector<PassedOption> options;
        const auto & extensions = config.at<jon::arr_t>("extensions");
        for (size_t i = 0; i < args.size(); i++) {
            const auto & arg = args.at(i);

            bool sourceFile = false;
            for (const auto & ext : extensions) {
                if (endsWith(arg, "." + ext.get<std::string>())) {
                    sourceFile = true;
                }
            }

            if (sourceFile) {
                if (entryFile.some()) {
                    throw std::runtime_error("Entry file cannot be specified twice");
                }
                entryFile = arg;
                continue;
            }

            // Flags start with `-`, it might be `--` or `-`, does not matter
            if (startsWith(arg, "-")) {
                // Check if we encountered first passed flag and set command to default if not specified
                if (passedCom.none()) {
                    commandDefaulted = true;
                    passedCom = getCommand(config.strAt("default-command"));
                }

                bool isAlias = not startsWith(arg, "--");

                for (const auto & flag : passedCom.unwrap().getFlags()) {

                }
            }

            // If it is not a source file or flag, then it might me a command
            if (passedCom.some()) {
                if (commandDefaulted) {
                    // Give different error message if command was defaulted, for readability :)
                    throw CLIError(
                        "Command defaulted to '" + passedCom.unwrap().getName()
                        + "', specify command '" + arg + "' as first argument");
                }
                throw CLIError("Command already specified as '" + passedCom.unwrap().getName() + "'");
            }
            passedCom = getCommand(arg);
        }
    }

    void CLI::loadConfig() {
        // TODO: Add config validation, unique flags, etc.

        config = jon::fromFile("./config.jon");

        for (const auto & j : config.at<jon::arr_t>("commands")) {
            commands.emplace(j.at<jon::str_t>("name"), j.as<Command>());
        }

        for (const auto & j : config.at("bool-options").arrAt("true")) {
            boolArgTrueValues.emplace_back(j.getStr());
        }

        for (const auto & j : config.at("bool-options").arrAt("false")) {
            boolArgFalseValues.emplace_back(j.getStr());
        }
    }
}
