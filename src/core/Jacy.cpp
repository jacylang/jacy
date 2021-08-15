#include "core/Jacy.h"

namespace jc::core {
    Jacy::Jacy() {}

    void Jacy::meow(int argc, const char ** argv) {
        cli::CLI cli;
        cli.applyArgs(argc, argv);
    }
}
