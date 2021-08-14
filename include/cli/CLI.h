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
        using options_t = std::vector<std::string>;
        using options_cnt_t = uint8_t;
        using deps_t = std::vector<std::string>;

        enum class Type {
            Bool,
            Str,
        };

        Flag(Type type, Option<options_cnt_t> valuesCount, const options_t & values, const deps_t & dependsOn)
            : type{type}, valuesCount{valuesCount}, values{values}, dependsOn{dependsOn} {}

        static Type typeFromString(const std::string & str) {
            if (str == "string") {
                return Type::Str;
            }
            if (str == "bool") {
                return Type::Bool;
            }

            throw CLIError("Invalid flag type '" + str + "'");
        }

        const Type type;
        const Option<options_cnt_t> valuesCount;
        const options_t values;
        const deps_t dependsOn;
    };

    class Command {
    public:
        using flags_t = std::vector<Flag>;

    public:
        Command(const flags_t & flags)
            : flags{flags} {}

    public:
        static Command fromJon(const jon & j) {
            using namespace utils::num;

            Command::flags_t flags;
            if (j.has("flags")) {
                for (const auto & flag : j.at<jon::arr_t>("flags")) {
                    const auto type = Flag::typeFromString(flag.at<jon::str_t>("type"));
                    Option<Flag::options_cnt_t> optionsCount{None};

                    if (flag.has("options-count")) {
                        const auto cnt = flag.at<jon::int_t>("options-count");
                        if (cnt > 0) {
                            optionsCount = safeAs<Flag::options_cnt_t, jon::int_t>(cnt);
                        }
                    }

                    Flag::options_t options;
                    if (flag.has("options")) {
                        for (const auto & opt : flag.at<jon::arr_t>("options")) {
                            options.emplace_back(opt.get<jon::str_t>());
                        }
                    }

                    Flag::deps_t deps;
                    if (flag.has("deps")) {
                        for (const auto & dep : flag.at<jon::arr_t>("deps")) {
                            deps.emplace_back(dep.get<jon::str_t>());
                        }
                    }

                    flags.emplace_back(type, optionsCount, options, deps);
                }
            }

            return Command {flags};
        }

    private:
        const flags_t flags;
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
