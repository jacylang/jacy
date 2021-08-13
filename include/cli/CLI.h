#ifndef JACY_CLI_H
#define JACY_CLI_H

/**
 * Jacy CLI Interface
 */

#include <string>
#include <map>
#include <vector>

#include "utils/str.h"
#include "utils/map.h"
#include "utils/arr.h"
#include "common/Error.h"
#include "log/Logger.h"
#include "cli/Args.h"
#include "jon/jon.h"

using jon = jacylang::jon;

namespace jc::cli {
    struct CLIError : common::Error {
        explicit CLIError(const std::string & msg) : Error(msg) {}
    };

    class Command {
    public:
        enum class Type {
            Bool,
            Str,
        };

        using flags_t = std::vector<std::string>;
        using deps_t = std::vector<std::string>;

    public:
        Command(Type type, const flags_t & flags, const deps_t & dependsOn)
            : type {type}, flags{flags}, dependsOn{dependsOn} {}

    public:
        static Command fromJon(const jon & j) {

        }

    private:
        const Type type;
        const flags_t flags;
        const deps_t dependsOn;
    };

    class CLI {
    public:
        CLI();
        ~CLI() = default;

        void applyArgs(int argc, const char ** argv);

        const Args & getConfig() const {
            return argsStorage;
        }

    private:
        // Note: This logger cannot be configure via cli
        log::Logger log{"cli"};

        Args argsStorage{};

        str_vec prepareArgs(int argc, const char ** argv);
        Option<bool> parseBool(const std::string & val);

    private:
        jon config;
        std::map<std::string, Command> commands;
        std::map<std::string, std::string> aliases;

        void loadConfig();

        // Storage //
    private:
        Option<std::string> entryFile{None};

    private:
        const static str_vec boolArgTrueValues;
        const static str_vec boolArgFalseValues;
    };
}

#endif // JACY_CLI_H
