#ifndef JACY_CLI_COMMANDS_COMMAND_LIST_H
#define JACY_CLI_COMMANDS_COMMAND_LIST_H

#include "cli/commands/Compile.h"

namespace jc::cli {
    class CommandList {
    public:
        CommandList() {
            list.emplace("compile", std::make_unique<Compile>());
        }

    private:
        std::map<std::string, command_ptr> list;
    };
}

#endif // JACY_CLI_COMMANDS_COMMAND_LIST_H
