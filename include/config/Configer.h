#ifndef JACY_CONFIG_CONFIGER_H
#define JACY_CONFIG_CONFIGER_H

#include "cli/CLICommand.h"
#include "config/Config.h"

namespace jc::config {
    class Configer {
    public:
        Configer() = default;

        void applyCLIArgs(const cli::PassedCommand & args);
    };
}

#endif // JACY_CONFIG_CONFIGER_H
