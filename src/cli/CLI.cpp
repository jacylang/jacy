#include "cli/CLI.h"

namespace jc::cli {
    CLI::CLI() {}

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


    }
}
