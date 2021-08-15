#ifndef JACY_CLI_COMMANDS_BASECOMMAND_H
#define JACY_CLI_COMMANDS_BASECOMMAND_H

#include <memory>

#include "cli/CLICommand.h"

namespace jc::cli {
    class BaseCommand;
    using command_ptr = std::unique_ptr<BaseCommand>;

    class BaseCommand {
    public:
        BaseCommand() = default;
        virtual ~BaseCommand() = default;

        virtual int run(PassedCommand && command) = 0;
    };
}

#endif // JACY_CLI_COMMANDS_BASECOMMAND_H
