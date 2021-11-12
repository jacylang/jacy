#ifndef JACY_CLI_COMMANDS_COMMAND_LIST_H
#define JACY_CLI_COMMANDS_COMMAND_LIST_H

#include "cli/commands/Compile.h"
#include "cli/commands/Help.h"

namespace jc::cli {
    class CommandList {
    public:
        CommandList() {
            list.emplace("compile", std::make_unique<Compile>());
            list.emplace("help", std::make_unique<Help>());
        }

        const auto & getList() const {
            return list;
        }

        const auto & tryGet(const std::string & name) const {
            const auto & found = list.find(name);
            if (found == list.end()) {
                throw std::logic_error(log::fmt("Unknown CLI command '", name, "'"));
            }
            return found->second;
        }

    private:
        std::map<std::string, BaseCommand::Ptr> list;
    };
}

#endif // JACY_CLI_COMMANDS_COMMAND_LIST_H
