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
        const Def & getDef(def_id defId) const {
            return defs.at(defId);
        }

        def_id define(Def && def) {
            defs.emplace_back(std::move(def));
            return defs.size() - 1;
        }

    private:
        std::vector<Def> defs;
    };
}

#endif // JACY_RESOLVE_DEFINITION_H
