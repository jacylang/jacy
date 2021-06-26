#ifndef JACY_RESOLVE_NAME_H
#define JACY_RESOLVE_NAME_H

#include <map>

#include "ast/Node.h"
#include "resolve/Module.h"

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
        opt_node_id define(const std::string & name, node_id nodeId);

        /// Resolves name in rib namespace
        /// Returns `None` if no `Name` found
        opt_node_id resolve(const std::string & name);

        void bindMod(module_ptr module);

        explicit Rib(Kind kind) : kind(kind) {}
    };
}

#endif // JACY_RESOLVE_NAME_H
