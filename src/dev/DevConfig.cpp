#include "dev/DevConfig.h"

namespace jc::dev {
    const std::map<std::string, bool> DevConfig::defaultBoolArgs = {
        {"dev", true}
    };
    const std::map<std::string, std::vector<std::string>> DevConfig::defaultKeyValueArgs = {
        {"print", {"ast", "tokens", "sugg", "source", "names"}},
        {"compile-depth", {"parser"}}
    };
}
