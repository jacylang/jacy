#ifndef JACY_CLI_COMMANDS_COMPILE_H
#define JACY_CLI_COMMANDS_COMPILE_H

#include "cli/commands/BaseCommand.h"

#include "common/Config.h"
#include "core/Interface.h"

namespace jc::cli {
    class Compile : public BaseCommand {
    public:
        Compile() = default;

        int run(PassedCommand && args) override;
    };
}

#endif // JACY_CLI_COMMANDS_COMPILE_H
