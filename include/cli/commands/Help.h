#ifndef JACY_CLI_COMMANDS_HELP_H
#define JACY_CLI_COMMANDS_HELP_H

#include "cli/commands/BaseCommand.h"

namespace jc::cli {
    class Help : public BaseCommand {
    public:
        int run(PassedCommand && args) override {
            std::cout << "HELP";
        }
    };
}

#endif // JACY_CLI_COMMANDS_HELP_H
