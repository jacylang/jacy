#ifndef JACY_CLI_CLICOMMAND_H
#define JACY_CLI_CLICOMMAND_H

#include <string>
#include <vector>
#include <cstdint>
#include <set>

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

    struct CLIFlag {
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

        CLIFlag(
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

        static CLIFlag fromJon(const jon & j) {
            using namespace utils::num;

            const auto type = CLIFlag::typeFromString(j.at<jon::str_t>("type"));
            Option<CLIFlag::value_count_t> valCount{None};

            if (j.has("value-count")) {
                const auto cnt = j.at<jon::int_t>("value-count");
                if (cnt > 0) {
                    valCount = safeAs<CLIFlag::value_count_t, jon::int_t>(cnt);
                }
            }

            CLIFlag::values_t values;
            if (j.has("values")) {
                for (const auto & val : j.at<jon::arr_t>("values")) {
                    values.emplace_back(val.get<jon::str_t>());
                }
            }

            CLIFlag::deps_t deps;
            if (j.has("deps")) {
                for (const auto & dep : j.at<jon::arr_t>("deps")) {
                    deps.emplace_back(dep.get<jon::str_t>());
                }
            }

            Duplication dupl{Duplication::Denied};

            if (j.has("duplicates")) {
                dupl = duplFromString(j.strAt("duplicates"));
            }

            return CLIFlag {j.strAt("name"), type, valCount, values, deps, dupl};
        }

        std::string name;
        Type type;
        Option<value_count_t> valuesCount;
        values_t values;
        deps_t dependsOn;
        Duplication duplication;
    };

    class CLICommand {
    public:
        using flags_t = std::vector<CLIFlag>;

    public:
        CLICommand(const std::string & name, const flags_t & flags)
            : name{name}, flags{flags} {}

    public:
        static CLICommand fromJon(const jon & j) {
            CLICommand::flags_t flags;

            if (j.has("flags")) {
                for (const auto & flag : j.at<jon::arr_t>("flags")) {
                    flags.emplace_back(flag.as<CLIFlag>());
                }
            }

            return CLICommand {j.strAt("name"), flags};
        }

        auto getName() const {
            return name;
        }

        auto getFlags() const {
            return flags;
        }

        Option<CLIFlag> findFlag(const std::string & name) const {
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

        auto getBool() const {
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

        PassedCommand(const flags_t & flags, const Option<std::string> & entryFile)
            : flags{flags}, entryFile{entryFile} {}

        const auto & getFlags() const {
            return flags;
        }

        const auto & getEntryFile() const {
            return entryFile;
        }

        auto checkBoolFlag(const std::string & name) const {
            const auto & found = flags.find(name);
            if (found == flags.end()) {
                return false;
            }
            if (found->second.kind() != PassedFlag::Kind::Bool) {
                throw std::logic_error("'" + name + "' is not a boolean flag");
            }
            return found->second.getBool();
        }

        Option<PassedFlag::values_t> getFlagValues(const std::string & name) const {
            const auto & found = flags.find(name);
            if (found == flags.end()) {
                return None;
            }
            if (found->second.kind() != PassedFlag::Kind::Str) {
                throw std::logic_error("'" + name + "' is not a string flag");
            }
            return found->second.getArgs();
        }

        Option<std::string> getFlagSingleValue(const std::string & name) const {
            const auto & found = flags.find(name);
            if (found == flags.end()) {
                return None;
            }
            if (found->second.kind() != PassedFlag::Kind::Str) {
                throw std::logic_error("'" + name + "' is not a string flag");
            }

            if (found->second.getArgs().size() != 1) {
                throw std::logic_error("Option '" + name + "' must have a single value or none");
            }

            return *found->second.getArgs().begin();
        }

    private:
        flags_t flags;
        Option<std::string> entryFile;
    };
}

#endif // JACY_CLI_CLICOMMAND_H
