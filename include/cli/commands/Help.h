#ifndef JACY_CLI_COMMANDS_HELP_H
#define JACY_CLI_COMMANDS_HELP_H

#include "cli/commands/BaseCommand.h"
#include "log/Logger.h"

namespace jc::cli {
    class Help : public BaseCommand {
    public:
        int run(PassedCommand && args) override {
            log.raw("USAGE").nl();
            log.raw(indent + 1, getConfig().at("help").strAt("basic-usage")).nl();

            const auto & comConfig = getConfig().at(args.getName());

            log.raw(comConfig.strAt("name"), " - ", comConfig.strAt("description")).nl();

            log.raw("OPTIONS").nl();
            incIndent();
            for (const auto & flag : getConfig().arrAt("flags")) {
                printIndent();
                log.raw(flag.strAt("name")).nl();
            }
            decIndent();

            return 0;
        }

    private:
        log::Logger log {"help-command"};

        log::Indent<2> indent {0};

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
