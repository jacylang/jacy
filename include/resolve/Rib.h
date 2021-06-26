#ifndef JACY_RESOLVE_RIB_H
#define JACY_RESOLVE_RIB_H

#include <map>

#include "ast/Node.h"
#include "resolve/Module.h"
#include "resolve/ResStorage.h"

namespace jc::resolve {
    using ast::node_id;
    using ast::opt_node_id;

    struct Rib;
    using rib_ptr = std::shared_ptr<Rib>;
    using opt_rib = dt::Option<rib_ptr>;
    using rib_stack = std::vector<rib_ptr>;

    struct Rib {
        enum class Kind {
            Raw,
            Root,
            Mod,
        } kind;

        std::map<std::string, node_id> locals;
        dt::Option<module_ptr> boundModule{dt::None};

        /// Define new local.
        /// Returns local node_id that was already defined if it was
        opt_node_id define(const ast::id_ptr & ident);

        /// Resolves name in rib namespace
        /// Returns `false` if failed to resolve a name, or sets resolution in case of success
        bool resolve(Namespace ns, const ast::id_ptr & ident, ResStorage & resStorage);

        void bindMod(module_ptr module);

        explicit Rib(Kind kind) : kind(kind) {}
    };
}

#endif // JACY_RESOLVE_RIB_H
