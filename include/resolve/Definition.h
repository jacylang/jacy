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
}

#endif // JACY_RESOLVE_DEFINITION_H
