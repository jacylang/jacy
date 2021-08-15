#ifndef JACY_CLI_COMMANDS_BASECOMMAND_H
#define JACY_CLI_COMMANDS_BASECOMMAND_H

namespace jc::cli {
    class BaseCommand {
    public:
        BaseCommand() {}

        virtual int run() = 0;
    };
}

#endif // JACY_CLI_COMMANDS_BASECOMMAND_H
