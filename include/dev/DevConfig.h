#ifndef JACY_DEVCONFIG_H
#define JACY_DEVCONFIG_H

/**
 * This a collection of rules that are hardcoded and used only for development and inside configuration parts.
 *
 * NOTE: The usage of JACY_DEBUG in compiler parts is not encouraged. Use it in configs!
 */

#include <map>
#include <vector>
#include <string>

namespace jc::dev {
    struct DevConfig {
        const static bool dev = true;

        const static std::map<std::string, bool> defaultBoolArgs;
        const static std::map<std::string, std::vector<std::string>> defaultKeyValueArgs;
    };
}

#endif // JACY_DEVCONFIG_H
