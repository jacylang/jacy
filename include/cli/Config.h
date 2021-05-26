#ifndef JACY_CONFIG_H
#define JACY_CONFIG_H

#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include "data_types/Option.h"
#include "utils/map.h"
#include "dev/DevConfig.h"

namespace jc::cli {
    using str_vec = std::vector<std::string>;
    using key_value_arg = std::pair<int, str_vec>; // Pair of count (-1 means any count) and values expected

    struct Config {
        // Allowed extensions for source files
        const static str_vec allowedExtensions;

        // Allowed boolean arguments (with '--' prefix)
        const static str_vec allowedBoolArgs;

        // Allowed key-value arguments (with '-' prefix and subsequent values)
        const static std::map<std::string, key_value_arg> allowedKeyValueArgs;

        // Key-value arguments that receive any non-specific parameters
        const static str_vec anyParamKeyValueArgs;

        // Default values for arguments //
        const static std::map<std::string, bool> defaultBoolArgs;
        const static std::map<std::string, str_vec> defaultKeyValueArgs;

        // Aliases (used as for boolean arguments, as for key-value arguments)
        const static std::map<std::string, std::string> aliases;

        // Some arguments can be set only if other argument is set, to check for it, here's dependencies
        // NOTE!: 'argsDependencies' contains this kind of structures: `key-value-arg: bool-arg` or `bool-arg: bool-arg`
        //  `key-value-arg: key-value-arg` now is not implemented (and maybe there's no need)
        // The dependency structure is `the 'first' can be specified if all of right dependencies are specified`
        const static std::map<std::string, str_vec> argsDependencies;

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

#endif // JACY_CONFIG_H
