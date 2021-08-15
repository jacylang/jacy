#include "core/Jacy.h"

namespace jc::core {
    Jacy::Jacy() {}

    void Jacy::meow(int argc, const char ** argv) {
        cli::CLI cli;
        cli.applyArgs(argc, argv);
        common::Config::getInstance().applyCliCommand(cli.getConfig());

        Interface interface; // Initialize interface here to allow do something in constructors after Config inited
        interface.compile();
    }
}
