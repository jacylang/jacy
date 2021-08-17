#ifndef JACY_CLI_COMMANDS_COMMAND_LIST_H
#define JACY_CLI_COMMANDS_COMMAND_LIST_H

#include "cli/commands/Compile.h"
#include "cli/commands/Help.h"

namespace jc::cli {
    class CommandList {
    public:
        CommandList() {
            list.emplace("compile", std::make_unique<Compile>());
            list.emplace("help", std::make_unique<Compile>());
        }

        const auto & getList() const {
            return list;
        }

    private:
        std::map<std::string, command_ptr> list;
    };
}

#endif // JACY_CLI_COMMANDS_COMMAND_LIST_H
