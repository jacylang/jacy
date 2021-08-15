#ifndef JACY_JACY_H
#define JACY_JACY_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "common/Error.h"
#include "utils/str.h"
#include "core/Interface.h"
#include "cli/CLI.h"
#include "common/Config.h"

// Thank u, MinGW...
// AAAAAAAAAAAAAAAAAAAAAAAAA
#undef interface

namespace jc::core {
    class Jacy {
    public:
        Jacy();
        ~Jacy() = default;

        int meow(int argc, const char ** argv);
    };
}

#endif // JACY_JACY_H
