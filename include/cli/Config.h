#ifndef JACY_CONFIG_H
#define JACY_CONFIG_H

#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include "utils/map.h"
#include "dev/DevConfig.h"

namespace jc::cli {
    using str_vec = std::vector<std::string>;
    struct KeyValueArg {
        int count;
        str_vec allowed;
    };

    struct Config {
        enum class Mode {
            Repl,
            Source,
        } mode;

        // Allowed extensions for source files
        const static str_vec allowedExtensions;

        // Allowed boolean arguments (with '--' prefix)
        const static str_vec allowedBoolArgs;

        // Allowed key-value arguments (with '-' prefix and subsequent values)
        const static std::map<std::string, KeyValueArg> allowedKeyValueArgs;

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

        const str_vec & getSourceFiles() const;

        std::map<std::string, bool> boolArgs;
        std::map<std::string, str_vec> keyValueArgs;
        str_vec sourceFiles;
    };
}

#endif // JACY_CONFIG_H
