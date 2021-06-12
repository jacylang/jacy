#include "core/Jacy.h"

namespace jc::core {
    Jacy::Jacy() {}

    void Jacy::meow(int argc, const char ** argv) {
        try {
            cli.applyArgs(argc, argv);
            common::Config::getInstance().applyCliConfig(cli.getConfig());
            Interface interface; // Initialize interface here to allow do something in constructors after Config inited
            interface.compile();
        } catch (common::Error & e) {
            log.error(e.message);
            return;
        }
    }
}
