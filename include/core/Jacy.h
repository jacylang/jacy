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

namespace jc::core {
    class Jacy {
    public:
        Jacy();
        ~Jacy() = default;

        void run(int argc, const char ** argv);
        void runRepl();
        void runSource();
        void runCode(const std::string & code);

    private:
        common::Logger log{"jacy", {}};
        Interface interface;
    };
}

#endif // JACY_JACY_H
