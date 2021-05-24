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
        {"print", {-1, {"ast", "tokens", "sugg", "source", "names"}}},
        {"compile-depth", {1, {"lexer", "parser", "linter", "name-resolution"}}},
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
        const auto & arg = keyValueArgs.find(key);
        if (arg == keyValueArgs.end()) {
            return false;
        }
        return std::find(arg->second.begin(), arg->second.end(), value) != arg->second.end();
    }

    bool Config::specified(const std::string & argName) const {
        return is(argName) or keyValueArgs.find(argName) != keyValueArgs.end();
    }

    dt::Option<const str_vec&> Config::getValues(const std::string & kvArgName) const {
        const auto & found = keyValueArgs.find(kvArgName);
        if (found == keyValueArgs.end()) {
            return dt::None;
        }
        return found->second;
    }

    dt::Option<const std::string&> Config::getSingleValue(const std::string & kvArgName) const {
        const auto & found = keyValueArgs.find(kvArgName);
        if (found == keyValueArgs.end() or found->second.empty()) {
            return dt::None;
        }
        if (found->second.size() > 1) {
            common::Logger::devPanic("Unexpected count for key-value cli argument '" + kvArgName + "', more than 1");
        }
        return found->second.at(0);
    }

    const std::string & Config::getRootFile() const {
        return rootFile;
    }
}
