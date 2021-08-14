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
#include "utils/num.h"
#include "common/Error.h"
#include "log/Logger.h"
#include "cli/Args.h"
#include "jon/jon.h"

using jon = jacylang::jon;

namespace jc::cli {
    struct CLIError : common::Error {
        explicit CLIError(const std::string & msg) : Error(msg) {}
    };

    struct Flag {
        using values_t = std::vector<std::string>;
        using value_count_t = uint8_t;
        using deps_t = std::vector<std::string>;

        enum class Type {
            Bool,
            Str,
        };

        Flag(
            const std::string & name,
            Type type,
            Option<value_count_t> valuesCount,
            const values_t & values,
            const deps_t & dependsOn
        ) : name{name},
            type{type},
            valuesCount{valuesCount},
            values{values},
            dependsOn{dependsOn} {}

        static Type typeFromString(const std::string & str) {
            if (str == "string") {
                return Type::Str;
            }
            if (str == "bool") {
                return Type::Bool;
            }

            throw CLIError("Invalid flag type '" + str + "'");
        }

        static Flag fromJon(const jon & j) {
            using namespace utils::num;

            const auto type = Flag::typeFromString(j.at<jon::str_t>("type"));
            Option<Flag::value_count_t> optionsCount{None};

            if (j.has("value-count")) {
                const auto cnt = j.at<jon::int_t>("value-count");
                if (cnt > 0) {
                    optionsCount = safeAs<Flag::value_count_t, jon::int_t>(cnt);
                }
            }

            Flag::values_t options;
            if (j.has("values")) {
                for (const auto & opt : j.at<jon::arr_t>("values")) {
                    options.emplace_back(opt.get<jon::str_t>());
                }
            }

            Flag::deps_t deps;
            if (j.has("deps")) {
                for (const auto & dep : j.at<jon::arr_t>("deps")) {
                    deps.emplace_back(dep.get<jon::str_t>());
                }
            }

            return Flag {j.strAt("name"), type, optionsCount, options, deps};
        }

        const std::string name;
        const Type type;
        const Option<value_count_t> valuesCount;
        const values_t values;
        const deps_t dependsOn;
    };

    class Command {
    public:
        using flags_t = std::vector<Flag>;

    public:
        Command(const std::string & name, const flags_t & flags)
            : name{name}, flags{flags} {}

    public:
        static Command fromJon(const jon & j) {
            Command::flags_t flags;

            if (j.has("flags")) {
                for (const auto & flag : j.at<jon::arr_t>("flags")) {
                    flags.emplace_back(flag.as<Flag>());
                }
            }

            return Command {j.strAt("name"), flags};
        }

        auto getName() const {
            return name;
        }

        auto getFlags() const {
            return flags;
        }

    private:
        std::string name;
        flags_t flags;
    };

    /// Flag passed to cli
    struct PassedFlag {
        enum class Kind {
            Bool,
            Str,
        };

        PassedFlag(const std::vector<std::string> & value) : value{value} {}
        PassedFlag(bool value) : value{value} {}

        Kind kind() const {
            if (value.index() == 0) {
                return Kind::Bool;
            }
            return Kind::Str;
        }

        std::variant<bool, std::vector<std::string>> value;
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
        const Command & getCommand(const std::string & name) const;

    private:
        jon config;
        std::map<std::string, Command> commands;

        void loadConfig();

        // Storage //
    private:
        Option<std::string> entryFile{None};

    private:
        str_vec boolArgTrueValues;
        str_vec boolArgFalseValues;
    };
}

#endif // JACY_CLI_H
