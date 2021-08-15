#include "cli/commands/Compile.h"

namespace jc::cli {
    int Compile::run(PassedCommand && args) {
        common::Config::getInstance().applyCLIArgs(args);

        // Initialize interface here to allow do something in constructors after Config inited
        core::Interface interface;
        interface.compile();

        return 1;
    }
}