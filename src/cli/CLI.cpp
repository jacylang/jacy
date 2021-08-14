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
            error("Command '", name, "' does not exists");
        }
        return found->second;
    }

    PassedCommand CLI::applyArgs(int argc, const char ** argv) {
        using namespace utils::str;

        const auto & args = prepareArgs(argc, argv);

        bool commandDefaulted = false;
        Option<Command> passedCom{None};
        PassedCommand::flags_t flags;

        const auto & extensions = config.at<jon::arr_t>("extensions");

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
                if (passedCom.none()) {
                    commandDefaulted = true;
                    passedCom = getCommand(config.strAt("default-command"));
                }

                bool isAlias = not startsWith(peek(), "--");

                // Get flag name removing `-` or `--`
                auto name = peek().substr(isAlias ? 1 : 2);
                auto flag = passedCom.unwrap().findFlag(name);

                if (flag.none()) {
                    error("Invalid option '", flag, "'");
                }

                advance();

                auto eqSign = skipOpt("=");

                PassedFlag::values_t values;
                while (not eof()) {
                    if (not startsWith(peek(), "-")) {
                        break;
                    }

                    values.emplace_back(peek());
                }

                if (values.empty() and eqSign) {
                    error("Expected value after `=`");
                }
            }

            // If it is not a source file or flag, then it might me a command
            if (passedCom.some()) {
                if (commandDefaulted) {
                    // Give different error message if command was defaulted, for readability :)
                    error(
                        "Command defaulted to '",
                        passedCom.unwrap().getName(),
                        "', specify command '",
                        arg,
                        "' as first argument");
                }
                error("Command already specified as '", passedCom.unwrap().getName(), "'");
            }
            passedCom = getCommand(arg);
            advance();
        }

        return PassedCommand {passedCom.unwrap().getName(), flags};
    }

    void CLI::loadConfig() {
        // TODO: Add config validation, unique flags, etc.

        config = jon::fromFile("./config.jon");

        for (const auto & j : config.at<jon::arr_t>("commands")) {
            commands.emplace(j.at<jon::str_t>("name"), j.as<Command>());
        }

        for (const auto & j : config.at("bool-values").arrAt("true")) {
            boolArgTrueValues.emplace_back(j.getStr());
        }

        for (const auto & j : config.at("bool-values").arrAt("false")) {
            boolArgFalseValues.emplace_back(j.getStr());
        }
    }
}
