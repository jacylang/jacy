#include "cli/CLI.h"

namespace jc::cli {
    CLI::CLI() {
        loadConfig();
    }

    std::vector<std::string> CLI::prepareArgs(int argc, const char ** argv) {
        using namespace utils::str;

        str_vec args;
        // Start from 1 to skip bin file path
        for (int i = 1; i < argc; i++) {
            const std::string arg(argv[i]);
            for (const auto & part : splitKeep(arg, getConfig().strAt("arg-delimiters"))) {
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

    const CLICommand & CLI::getCommand(const std::string & name) const {
        auto found = configCommands.find(name);
        if (found == configCommands.end()) {
            error("Command '", name, "' does not exists");
        }
        return found->second;
    }

    int CLI::applyArgs(int argc, const char ** argv) {
        using namespace utils::str;
        using namespace utils::arr;

        auto args = prepareArgs(argc, argv);

        bool commandDefaulted = false;
        Option<CLICommand> command{None};
        PassedCommand::flags_t passedFlags;

        auto extensions = getConfig().arrAt("extensions");

        // Simple parser-like interface
        size_t index = 0;

        auto skipOpt = [&](const std::string & str) -> bool {
            if (args.at(index) == str) {
                index++;
                return true;
            }
            return false;
        };

        auto advance = [&]() -> void {
            index++;
        };

        auto peek = [&]() {
            return args.at(index);
        };

        auto eof = [&]() {
            return index >= args.size();
        };

        while (not eof()) {
            const auto & arg = peek();

            bool sourceFile = false;
            for (const auto & ext : extensions) {
                if (endsWith(arg, "." + ext.get<std::string>())) {
                    sourceFile = true;
                }
            }

            if (sourceFile) {
                if (entryFile.some()) {
                    error("Entry file cannot be specified twice");
                }
                entryFile = arg;
                advance();
                continue;
            }

            // Flags start with `-`, it might be `--` or `-`, does not matter
            if (startsWith(arg, "-")) {
                // Check if we encountered first passed flag and set command to default if not specified
                if (command.none()) {
                    commandDefaulted = true;
                    command = getCommand(getConfig().strAt("default-command"));
                }

                bool isAlias = not startsWith(peek(), "--");

                // Get flag name removing `-` or `--`
                auto name = peek().substr(isAlias ? 1 : 2);
                auto uncheckedFlag = command.unwrap().findFlag(name);

                // TODO: Allow common-flags
                if (uncheckedFlag.none()) {
                    error("Invalid option '", peek(), "'");
                }

                auto flag = uncheckedFlag.unwrap();

                advance();

                auto eqSign = skipOpt("=");

                if (flag.type == CLIFlag::Type::Bool and eqSign) {
                    // Parse boolean option value
                    auto val = parseBool(peek());

                    if (val.none()) {
                        error("Invalid value for boolean option '", flag.name, "' - '", peek(), "'");
                    }

                    advance(); // Skip bool value

                    auto addResult = passedFlags.emplace(name, val.unwrap());

                    if (not addResult.second) {
                        if (flag.duplication == CLIFlag::Duplication::Denied) {
                            error("Duplicate option '", flag.name, "'");
                        }
                        if (flag.duplication == CLIFlag::Duplication::Merge) {
                            // Update value of boolean option
                            addResult.first->second.value = val.unwrap();
                        }
                    }
                } else {
                    // Parse key-value option values
                    PassedFlag::values_t values;

                    while (not eof()) {
                        if (startsWith(peek(), "-")) {
                            break;
                        }

                        if (peek() == ",") {
                            advance();
                            continue;
                        }

                        if (not has(flag.values, peek())) {
                            error("Invalid value for '", flag.name, "' - '", peek(), "'");
                        }

                        values.emplace(peek());
                    }

                    if (values.empty() and eqSign) {
                        error("Expected value after `=`");
                    }

                    if (flag.valuesCount.some() and values.size() != flag.valuesCount.unwrap()) {
                        error(
                            "Option '",
                            flag.name,
                            "' requires ",
                            flag.valuesCount.unwrap(),
                            ", ",
                            values.size(),
                            " given");
                    }

                    auto addResult = passedFlags.emplace(name, std::move(values));

                    if (not addResult.second) {
                        if (flag.duplication == CLIFlag::Duplication::Denied) {
                            error("Duplicate option '", flag.name, "'");
                        }
                        if (flag.duplication == CLIFlag::Duplication::Merge) {
                            addResult.first->second.getArgs().insert(values.begin(), values.end());
                        }
                    }
                }
            } else {
                // TODO[Important]: Check if file is word-like

                // If it is not a source file or flag, then it might me a command
                if (command.some()) {
                    if (commandDefaulted) {
                        // Give different error message if command was defaulted, for readability :)
                        error(
                            "Command defaulted to '",
                            command.unwrap().getName(),
                            "', specify command '",
                            arg,
                            "' as first argument");
                    }
                    error("Command already specified as '", command.unwrap().getName(), "'");
                }
                std::cout << "command name: " << getConfig().strAt("default-command") << std::endl;
                command = getCommand(arg);
                advance();
            }
        }

        // Post-check flags
        for (const auto & flag : command.unwrap().getFlags()) {
            // Check if flag is passed
            if (passedFlags.find(flag.name) != passedFlags.end()) {
                // Check if flag depends on some other
                for (const auto & dep : flag.dependsOn) {
                    if (passedFlags.find(dep) == passedFlags.end()) {
                        error("Flag '", flag.name, "' requires '", dep, "' to be specified");
                    }
                }
            }
        }

        return commandList.getList().at(command.unwrap().getName())->run({passedFlags, entryFile});
    }

    void CLI::loadConfig() {
        // TODO: Add config validation, unique flags, etc.

        const auto & config = getConfig();

        for (const auto & j : config.arrAt("commands")) {
            configCommands.emplace(j.strAt("name"), j.as<CLICommand>());
        }

        for (const auto & j : config.at("bool-values").arrAt("true")) {
            boolArgTrueValues.emplace_back(j.getStr());
        }

        for (const auto & j : config.at("bool-values").arrAt("false")) {
            boolArgFalseValues.emplace_back(j.getStr());
        }
    }
}
