#include "cli/CLI.h"

namespace jc::cli {
    CLI::CLI() {
        loadConfig();
    }

    std::vector<std::string> CLI::prepareArgs(int argc, const char ** argv) {
        using namespace utils::str;

        StrVec args;
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

    Option<CLIFlag> CLI::findCommonFlag(const std::string & name) const {
        for (const auto & flag : getConfig().arrAt("common-flags")) {
            if (flag.strAt("name") == name) {
                return flag.as<CLIFlag>();
            }
        }
        return None;
    }

    int CLI::applyArgs(int argc, const char ** argv) {
        using namespace utils::str;
        using namespace utils::arr;

        auto args = prepareArgs(argc, argv);

        Option<std::string> magicFlag = None;
        bool commandDefaulted = false;
        Option<CLICommand> command = None;
        PassedCommand::FlagList passedFlags;

        auto extensions = getConfig().arrAt("extensions");

        // Simple parser-like interface
        size_t index = 0;

        auto eof = [&]() {
            return index >= args.size();
        };

        auto skipOpt = [&](const std::string & str) -> bool {
            if (not eof() and args.at(index) == str) {
                index++;
                return true;
            }
            return false;
        };

        auto advance = [&]() -> void {
            index++;
        };

        auto peek = [&]() -> std::string {
            return args.at(index);
        };

        auto trySetDefaultCommand = [&]() -> void {
            if (command.none()) {
                commandDefaulted = true;
                command = getCommand(getConfig().strAt("default-command"));
            }
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
                advance(); // Skip file
                continue;
            }

            // Flags start with `-`, it might be `--` or `-`, does not matter
            if (startsWith(arg, "-")) {
                // Check if we encountered first passed flag and set command to default if not specified
                trySetDefaultCommand();

                bool isAlias = not startsWith(peek(), "--");

                // Get flag name removing `-` or `--`
                auto name = peek().substr(isAlias ? 1 : 2);
                auto uncheckedFlag = command.unwrap().findFlag(name);

                if (uncheckedFlag.none()) {
                    auto commonFlag = findCommonFlag(name);
                    if (commonFlag.none()) {
                        error("Invalid option '", peek(), "'");
                    }
                    uncheckedFlag = commonFlag;
                }

                auto flag = uncheckedFlag.unwrap();

                if (flag.magicMethod.some()) {
                    magicFlag = flag.name;
                }

                advance(); // Skip option

                auto eqSign = skipOpt("=");

                if (flag.type == CLIFlag::Type::Bool) {
                    bool val = true;
                    if (eqSign) {
                        // Parse boolean option value
                        auto parsedVal = parseBool(peek());

                        if (parsedVal.none()) {
                            error("Invalid value for boolean option '", flag.name, "' - '", peek(), "'");
                        }

                        advance(); // Skip bool value
                        val = parsedVal.unwrap();
                    }

                    auto addResult = passedFlags.emplace(name, val);

                    if (not addResult.second) {
                        if (flag.duplication == CLIFlag::Duplication::Denied) {
                            error("Duplicate option '", flag.name, "'");
                        }
                        if (flag.duplication == CLIFlag::Duplication::Merge) {
                            // Update value of boolean option
                            addResult.first->second.value = val;
                        }
                    }
                } else {
                    // Parse key-value option values
                    PassedFlag::ValueList values;

                    while (not eof()) {
                        if (startsWith(peek(), "-")) {
                            break;
                        }

                        if (not has(flag.values, peek())) {
                            error("Invalid value for '", flag.name, "' - '", peek(), "'");
                        }

                        values.emplace(peek());
                        advance();

                        if (not eof() and peek() == ",") {
                            advance();
                            continue;
                        }

                        break;
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

                // If it is not a source file or flag, then it might be a command
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
                command = getCommand(arg);
                advance();
            }
        }

        if (command.none()) {
            trySetDefaultCommand();
        }

        // Post-check flags
        for (const auto & flag : command.unwrap().getFlags()) {
            // Check if flag is passed
            if (passedFlags.find(flag.name) != passedFlags.end()) {
                // Check if flag depends on some other
                for (const auto & dep : flag.dependsOn) {
                    if (passedFlags.find(dep) == passedFlags.end()) {
                        error("Option '", flag.name, "' requires '", dep, "' to be specified");
                    }
                }

                // Check if flag denies usage of another flag
                for (const auto & excl : flag.excludes) {
                    if (passedFlags.find(excl) != passedFlags.end()) {
                        error("Option '", flag.name, "' cannot be used together with '", excl, "'");
                    }
                }
            }
        }

        // Note: Magic methods invert functionality -- they are called as commands

        std::string commandName = command.unwrap().getName();

        if (magicFlag.some()) {
            commandName = magicFlag.unwrap();
        }

        // Note: We pass here `command` name, but not `commandName` because `commandName` is important for magic flags
        return commandList.tryGet(commandName)->run(PassedCommand {
            command.unwrap().getName(),
            passedFlags,
            entryFile
        });
    }

    void CLI::loadConfig() {
        // TODO: Add config validation, unique flags, etc.

        const auto & config = getConfig();

        for (const auto & j : config.objAt("commands")) {
            configCommands.emplace(j.first, j.second.as<CLICommand>());
        }

        for (const auto & j : config.at("bool-values").arrAt("true")) {
            boolArgTrueValues.emplace_back(j.getStr());
        }

        for (const auto & j : config.at("bool-values").arrAt("false")) {
            boolArgFalseValues.emplace_back(j.getStr());
        }
    }
}
