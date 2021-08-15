#ifndef JACY_CLI_COMMAND_H
#define JACY_CLI_COMMAND_H

#include <string>
#include <vector>
#include <cstdint>

#include "common/Error.h"
#include "utils/str.h"
#include "utils/map.h"
#include "utils/arr.h"
#include "utils/num.h"
#include "data_types/Option.h"
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

        /// Possible kinds of duplications allowed for flag
        enum class Duplication {
            Denied,
            Merge,
        };

        enum class Type {
            Bool,
            Str,
        };

        Flag(
            const std::string & name,
            Type type,
            Option<value_count_t> valuesCount,
            const values_t & values,
            const deps_t & dependsOn,
            Duplication duplication
        ) : name{name},
            type{type},
            valuesCount{valuesCount},
            values{values},
            dependsOn{dependsOn},
            duplication{duplication} {}

        static Type typeFromString(const std::string & str) {
            if (str == "string") {
                return Type::Str;
            }
            if (str == "bool") {
                return Type::Bool;
            }

            throw CLIError("Invalid flag type '" + str + "'");
        }

        static Duplication duplFromString(const std::string & str) {
            if (str == "denied") {
                return Duplication::Denied;
            }

            if (str == "merge") {
                return Duplication::Merge;
            }

            throw std::logic_error("Invalid `duplicates` value");
        }

        static Flag fromJon(const jon & j) {
            using namespace utils::num;

            const auto type = Flag::typeFromString(j.at<jon::str_t>("type"));
            Option<Flag::value_count_t> valCount{None};

            if (j.has("value-count")) {
                const auto cnt = j.at<jon::int_t>("value-count");
                if (cnt > 0) {
                    valCount = safeAs<Flag::value_count_t, jon::int_t>(cnt);
                }
            }

            Flag::values_t values;
            if (j.has("values")) {
                for (const auto & val : j.at<jon::arr_t>("values")) {
                    values.emplace_back(val.get<jon::str_t>());
                }
            }

            Flag::deps_t deps;
            if (j.has("deps")) {
                for (const auto & dep : j.at<jon::arr_t>("deps")) {
                    deps.emplace_back(dep.get<jon::str_t>());
                }
            }

            Duplication dupl{Duplication::Denied};

            if (j.has("duplicates")) {
                dupl = duplFromString(j.strAt("duplicates"));
            }

            return Flag {j.strAt("name"), type, valCount, values, deps, dupl};
        }

        const std::string name;
        const Type type;
        const Option<value_count_t> valuesCount;
        const values_t values;
        const deps_t dependsOn;
        const Duplication duplication;
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

        Option<Flag> findFlag(const std::string & name) const {
            for (const auto & flag : flags) {
                if (flag.name == name) {
                    return flag;
                }
            }
            return None;
        }

    private:
        std::string name;
        flags_t flags;
    };

    /// Flag passed to cli
    struct PassedFlag {
        using values_t = std::set<std::string>;

        enum class Kind {
            Bool,
            Str,
        };

        PassedFlag(values_t && value) : value{std::move(value)} {}
        PassedFlag(bool value) : value{value} {}

        Kind kind() const {
            if (value.index() == 0) {
                return Kind::Bool;
            }
            return Kind::Str;
        }

        auto getBool() {
            return std::get<bool>(value);
        }

        const auto & getArgs() const {
            return std::get<values_t>(value);
        }

        auto & getArgs() {
            return std::get<values_t>(value);
        }

        std::variant<bool, values_t> value;
    };

    struct PassedCommand {
        using flags_t = std::map<std::string, PassedFlag>;

        PassedCommand(const std::string & name, const flags_t & flags, const Option<std::string> & entryFile)
            : name{name}, flags{flags}, entryFile{entryFile} {}

        std::string name;
        flags_t flags;
        Option<std::string> entryFile;
    };
}

#endif // JACY_CLI_COMMAND_H