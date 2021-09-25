#include "core/Jacy.h"

namespace jc::core {
    Jacy::Jacy() = default;

    int Jacy::meow(int argc, const char ** argv) {
        cli::CLI cli;

        return cli.applyArgs(argc, argv);
    }
}
