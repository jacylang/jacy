#include "cli/commands/Compile.h"

namespace jc::cli {
    int Compile::run(PassedCommand && command) {
        common::Config::getInstance().applyCliCommand(cli.getConfig());

        // Initialize interface here to allow do something in constructors after Config inited
        core::Interface interface;
        interface.compile();
    }
}
