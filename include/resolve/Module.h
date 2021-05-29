#ifndef JACY_RESOLVE_MODULESTACK_H
#define JACY_RESOLVE_MODULESTACK_H

#include "ast/Party.h"

namespace jc::resolve {
    struct Module;
    using module_ptr = std::unique_ptr<Module>;
    using module_stack = std::vector<module_ptr>;

    struct Module {
        enum class Kind {
            Dir,
            File,
            Mod,
        } kind;

        std::string name;

        Module(Kind kind, const std::string & name) : kind(kind), name(name) {}
    };
}

#endif // JACY_RESOLVE_MODULESTACK_H
