#include "cli/Config.h"

#include <tuple>

namespace jc::cli {
    // Settings //
    const str_vec Config::allowedExtensions = {
        "jc",
    };

    const str_vec Config::allowedBoolArgs = {
        "dev",
    };

    const std::map<std::string, key_value_arg> Config::allowedKeyValueArgs = {
        {"print", {-1, {"ast", "tokens", "sugg", "source"}}},
        {"compile-depth", {1, {"lexer", "parser", "linter"}}},
    };

    const str_vec Config::anyParamKeyValueArgs = {};


    const std::map<std::string, std::string> Config::aliases = {};

    const std::map<std::string, str_vec> Config::argsDependencies = {
        {"compile-depth", {"dev"}}, // Allow compilation depth set only if 'dev' is set
    };


    const std::map<std::string, bool> Config::defaultBoolArgs = {
        {"dev", false},
    };

    const std::map<std::string, str_vec> Config::defaultKeyValueArgs = {};


    // Checkers //
    bool Config::is(const std::string & argName) const {
        // At first we find, because we can get `map::at` error if argName does not exists in boolArgs
        return boolArgs.find(argName) != boolArgs.end() and boolArgs.at(argName);
    }

    bool Config::has(const std::string & key, const std::string & value) const {
        const auto & arg = keyValueArgs.at(key);
        return std::find(arg.begin(), arg.end(), value) != arg.end();
    }

    bool Config::specified(const std::string & argName) const {
        return is(argName) or keyValueArgs.find(argName) != keyValueArgs.end();
    }


    const str_vec & Config::getSourceFiles() const {
        return sourceFiles;
    }
}
