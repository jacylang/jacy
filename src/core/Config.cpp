#include "common/Config.h"

namespace jc::common {
    Config::Config() = default;

    void Config::applyCliConfig(const cli::Config & cliConfig) {
        // Apply key-value args //

        // `print`
        if (cliConfig.is())

        // Apply bool args //
        dev = cliConfig.is("dev");
    }
}
