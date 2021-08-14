#include "cli/CLI.h"

namespace jc::cli {
    CLI::CLI() {}

    const str_vec CLI::boolArgTrueValues = {
        "yes",
        "y",
        "true",
        "1",
        "on",
    };

    const str_vec CLI::boolArgFalseValues = {
        "no",
        "n",
        "false",
        "0",
        "off",
    };

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

    void CLI::applyArgs(int argc, const char ** argv) {
        loadConfig();

        using namespace utils::str;

        const auto & args = prepareArgs(argc, argv);

        const auto & extensions = config.at<jon::arr_t>("extensions");
        for (const auto & arg : args) {
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
        }
    }

    void CLI::loadConfig() {

        config = jon::fromFile("./config.jon");

        for (const auto & j : config.at<jon::arr_t>("commands")) {
            commands.emplace(j.at<jon::str_t>("name"), j.as<Command>());
        }
    }
}
