#ifndef JACY_CLI_COMMANDS_HELP_H
#define JACY_CLI_COMMANDS_HELP_H

#include "cli/commands/BaseCommand.h"
#include "cli/config.h"

#include "log/Logger.h"

namespace jc::cli {
    class Help : public BaseCommand {
    public:
        int run(PassedCommand && args) override {
            log.raw("Commands:").nl();
            for (const auto & command : getConfig().arrAt("commands")) {

            }

            return 0;
        }

    private:
        log::Logger log;

        log::Indent<2> indent;

        void printIndent() {
            log.raw(indent);
        }

        void incIndent() {
            indent = indent + 1;
        }

        void decIndent() {
            indent = indent - 1;
        }
    };
}

#endif // JACY_CLI_COMMANDS_HELP_H
