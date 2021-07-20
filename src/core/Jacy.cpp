#include "core/Jacy.h"

namespace jc::core {
    Jacy::Jacy() {}

    int Jacy::meow(int argc, const char ** argv) {
        try {
            cli::CLI cli;
            cli.applyArgs(argc, argv);
            log::Config::getInstance().applyCliConfig(cli.getConfig());

            Interface interface; // Initialize interface here to allow do something in constructors after Config inited
            interface.compile();
            return 0;
        } catch (log::Error & e) {
            log::Logger log{"jacy"};
            log.error(e.message);
            if (log::Config::getInstance().checkDev()) {
                throw e;
            }
            return 1;
        }
    }
}
