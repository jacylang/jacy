#ifndef JACY_CLI_COMMANDS_HELP_H
#define JACY_CLI_COMMANDS_HELP_H

#include "cli/commands/BaseCommand.h"
#include "log/Logger.h"

namespace jc::cli {
    class Help : public BaseCommand {
    public:
        int run(PassedCommand && args) override {
            using namespace utils::arr;
            using namespace utils::map;

            const auto & comConfig = getConfig().at("commands").at(args.getName());

            log.raw(comConfig.strAt("name"), " - ", comConfig.strAt("description")).nl();
            log.nl();

            log.raw("Other commands: ", utils::arr::join(utils::map::keys(getConfig().objAt("commands")), ", ")).nl();

            log.raw("USAGE").nl();
            log.raw(indent + 1, getConfig().at("help").strAt("basic-usage")).nl();
            log.nl();

            log.raw("OPTIONS").nl();
            incIndent();
            for (const auto & flag : comConfig.arrAt("flags")) {
                printIndent();
                log.raw(flag.strAt("name"));
                if (flag.has("description")) {
                    log.raw(" - ", flag.strAt("description"));
                }
                log.nl();
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
