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
        using ValueList = std::vector<std::string>;
        using ValueCount = uint8_t;
        using Deps = std::vector<std::string>;
        using Exclusions = std::vector<std::string>;

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
            Option<ValueCount> valuesCount,
            const ValueList & values,
            const Deps & dependsOn,
            Duplication duplication,
            Option<std::string> magicMethod
        ) : name{name},
            type{type},
            valuesCount{valuesCount},
            values{values},
            dependsOn{dependsOn},
            duplication{duplication},
            magicMethod{magicMethod} {}

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

            const auto type = CLIFlag::typeFromString(j.strAt("type"));
            Option<CLIFlag::ValueCount> valCount = None;

            // TODO: Check `value-count`
            if (j.has("value-count")) {
                const auto cnt = j.intAt("value-count");
                if (cnt > 0) {
                    valCount = safeAs<CLIFlag::ValueCount, jon::int_t>(cnt);
                }
            }

            CLIFlag::ValueList values;
            if (j.has("values")) {
                for (const auto & val : j.arrAt("values")) {
                    values.emplace_back(val.getStr());
                }
            }

            CLIFlag::Deps deps;
            if (j.has("deps")) {
                for (const auto & dep : j.arrAt("deps")) {
                    deps.emplace_back(dep.getStr());
                }
            }

            Exclusions excludes;
            if (j.has("excludes")) {
                for (const auto & exclusion : j.arrAt("exclusions")) {
                    excludes.emplace_back(exclusion.getStr());
                }
            }

            Duplication dupl{Duplication::Denied};

            if (j.has("duplicates")) {
                dupl = duplFromString(j.strAt("duplicates"));
            }

            Option<std::string> magicMethod = None;

            if (j.has("magic-method")) {
                magicMethod = j.strAt("magic-method");
            }

            return CLIFlag {j.strAt("name"), type, valCount, values, deps, dupl, magicMethod};
        }

        std::string name;
        Type type;
        Option<ValueCount> valuesCount;
        ValueList values;
        Deps dependsOn;
        Exclusions excludes;
        Duplication duplication;
        Option<std::string> magicMethod;
    };

    class CLICommand {
    public:
        using FlagList = std::vector<CLIFlag>;

    public:
        CLICommand(const std::string & name, const FlagList & flags)
            : name{name}, flags{flags} {}

    public:
        static CLICommand fromJon(const jon & j) {
            CLICommand::FlagList flags;

            if (j.has("flags")) {
                for (const auto & flag : j.arrAt("flags")) {
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
        FlagList flags;
    };

    /// Flag passed to cli
    struct PassedFlag {
        using ValueList = std::set<std::string>;

        enum class Kind {
            Bool,
            Str,
        };

        PassedFlag(ValueList && value) : value{std::move(value)} {}
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
            return std::get<ValueList>(value);
        }

        auto & getArgs() {
            return std::get<ValueList>(value);
        }

        std::variant<bool, ValueList> value;
    };

    struct PassedCommand {
        using FlagList = std::map<std::string, PassedFlag>;

        PassedCommand(const std::string & name, const FlagList & flags, const Option<std::string> & entryFile)
            : name{name}, flags{flags}, entryFile{entryFile} {}

        const auto & getFlags() const {
            return flags;
        }

        const auto & getEntryFile() const {
            return entryFile;
        }

        const auto & getName() const {
            return name;
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

        Option<PassedFlag::ValueList> getFlagValues(const std::string & name) const {
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
        std::string name;
        FlagList flags;
        Option<std::string> entryFile;
    };
}

#endif // JACY_CLI_CLICOMMAND_H
