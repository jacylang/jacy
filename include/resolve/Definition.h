#ifndef JACY_RESOLVE_DEFINITION_H
#define JACY_RESOLVE_DEFINITION_H

#include "span/Span.h"

namespace jc::resolve {
    using def_id = size_t;

    struct Def {
        enum class Kind {
            Mod,
            Func,
        };

        const span::Span span;
    };

    struct DefStorage {
        std::vector<Def> defs;
    };
}

#endif // JACY_RESOLVE_DEFINITION_H
