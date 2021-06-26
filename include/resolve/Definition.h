#ifndef JACY_RESOLVE_DEFINITION_H
#define JACY_RESOLVE_DEFINITION_H

#include "span/Span.h"
#include "ast/Node.h"

namespace jc::resolve {
    struct Module;
    using module_ptr = std::shared_ptr<Module>;
    using def_id = size_t;

    enum class DefKind {
        Root,
        File,
        Dir,

        Mod,
        Func,
        Const,
        Struct,
        Trait,
        TypeParam,
        Lifetime,
        ConstParam,
        Enum,
        Variant,
        TypeAlias,
        Impl,
    };

    struct Def {
        Def(DefKind kind, const span::Span & span) : kind(kind), span(span) {}

        DefKind kind;
        const span::Span span;
    };

    struct DefStorage {
        const Def & getDef(def_id defId) const {
            return defs.at(defId);
        }

        template<class ...Args>
        def_id define(Args ...args) {
            defs.emplace_back(Def(args...));
            return defs.size() - 1;
        }

    private:
        std::vector<Def> defs;
    };
}

#endif // JACY_RESOLVE_DEFINITION_H
