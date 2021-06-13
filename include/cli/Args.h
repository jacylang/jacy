#ifndef JACY_CLI_ARGS_H
#define JACY_CLI_ARGS_H

#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include "data_types/Option.h"
#include "utils/map.h"

namespace jc::cli {
    using str_vec = std::vector<std::string>;
    // Pair of count (None means any count) and values expected
    using key_value_arg = std::pair<dt::Option<size_t>, str_vec>;

    struct Args {
        const static std::string delimiters;

        // Allowed extensions for source files
        const static str_vec allowedExtensions;

        // Allowed boolean arguments (with '--' prefix)
        const static str_vec allowedBoolArgs;

        // Allowed key-value arguments (with '-' prefix and subsequent values)
        const static std::map<std::string, key_value_arg> allowedKeyValueArgs;

        // Key-value arguments that receive any non-specific parameters
        const static str_vec anyParamKeyValueArgs;

        // Aliases (used as for boolean arguments, as for key-value arguments)
        const static std::map<std::string, std::string> aliases;

        // Some arguments can be set only if other argument is set, to check for it, here's dependencies
        // NOTE!: 'argsDependencies' contains this kind of structures: `key-value-arg: bool-arg` or `bool-arg: bool-arg`
        //  `key-value-arg: key-value-arg` now is not implemented (and maybe there's no need)
        // The dependency structure is `the 'first' can be specified if all of right dependencies are specified`
        const static std::map<std::string, str_vec> argsDependencies;

        const static str_vec boolArgTrueValues;
        const static str_vec boolArgFalseValues;

        // Check for boolean arg
        bool is(const std::string & argName) const;

        // Check for set key-value argument element
        bool has(const std::string & key, const std::string & value) const;

        // Check if key-value argument is specified or bool-arg is true
        bool specified(const std::string & argName) const;

        dt::Option<str_vec> getValues(const std::string & kvArgName) const;
        dt::Option<std::string> getSingleValue(const std::string & kvArgName) const;

        const std::string & getRootFile() const;

        std::map<std::string, bool> boolArgs;
        std::map<std::string, str_vec> keyValueArgs;
        std::string rootFile;
    };
}

#endif // JACY_CLI_ARGS_H
