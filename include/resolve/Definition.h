#ifndef JACY_RESOLVE_DEFINITION_H
#define JACY_RESOLVE_DEFINITION_H

#include "span/Span.h"
#include "ast/Node.h"

namespace jc::resolve {
    struct Module;
    using module_ptr = std::shared_ptr<Module>;
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
        std::map<ast::node_id, module_ptr> anonBlocks;
    };
}

#endif // JACY_RESOLVE_DEFINITION_H
