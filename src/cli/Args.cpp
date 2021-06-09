#include "cli/Args.h"

namespace jc::cli {
    // Settings //
    const std::string Args::delimiters{"=,"};

    const str_vec Args::allowedExtensions = {
        "jc",
    };

    const str_vec Args::allowedBoolArgs = {
        "dev",
    };

    const std::map<std::string, key_value_arg> Args::allowedKeyValueArgs = {
        {"print", {dt::None, {"dir-tree", "tokens", "ast", "sugg", "source", "names", "all"}}},
        {"compile-depth", {1, {"parser", "name-resolution"}}},
        {"benchmark", {1, {"each-stage", "final"}}},
    };

    const str_vec Args::anyParamKeyValueArgs = {};

    const std::map<std::string, std::string> Args::aliases = {};

    const std::map<std::string, str_vec> Args::argsDependencies = {
        {"compile-depth", {"dev"}}, // Allow compilation depth set only if 'dev' is set
    };

    // Checkers //
    bool Args::is(const std::string & argName) const {
        // At first we find, because we can get `map::at` error if argName does not exists in boolArgs
        return boolArgs.find(argName) != boolArgs.end() and boolArgs.at(argName);
    }

    bool Args::has(const std::string & key, const std::string & value) const {
        const auto & arg = keyValueArgs.find(key);
        if (arg == keyValueArgs.end()) {
            return false;
        }
        return std::find(arg->second.begin(), arg->second.end(), value) != arg->second.end();
    }

    bool Args::specified(const std::string & argName) const {
        return is(argName) or keyValueArgs.find(argName) != keyValueArgs.end();
    }

    dt::Option<str_vec> Args::getValues(const std::string & kvArgName) const {
        // FIXME: Strange dev-only check
        if (allowedKeyValueArgs.find(kvArgName) == allowedKeyValueArgs.end()) {
            throw std::logic_error("Called `Args::getSingleValue` with unknown key-value argument `" + kvArgName + "`");
        }
        const auto & found = keyValueArgs.find(kvArgName);
        if (found == keyValueArgs.end()) {
            return dt::None;
        }
        return found->second;
    }

    dt::Option<std::string> Args::getSingleValue(const std::string & kvArgName) const {
        // FIXME: Strange dev-only check
        if (allowedKeyValueArgs.find(kvArgName) == allowedKeyValueArgs.end()) {
            throw std::logic_error("Called `Args::getSingleValue` with unknown key-value argument `" + kvArgName + "`");
        }
        const auto & found = keyValueArgs.find(kvArgName);
        if (found == keyValueArgs.end() or found->second.empty()) {
            return dt::None;
        }
        if (found->second.size() > 1) {
            throw std::logic_error("Unexpected count for key-value cli argument '" + kvArgName + "', more than 1");
        }
        return found->second.at(0);
    }

    const std::string & Args::getRootFile() const {
        return rootFile;
    }
}
