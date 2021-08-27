#ifndef JACY_RESOLVE_RIB_H
#define JACY_RESOLVE_RIB_H

#include <map>

#include "ast/Node.h"
#include "resolve/Module.h"
#include "resolve/ResStorage.h"

namespace jc::resolve {
    using ast::NodeId;

    struct Rib {
        using Ptr = std::shared_ptr<Rib>;
        using OptPtr = Option<Ptr>;
        using Stack = std::vector<Ptr>;

        enum class Kind {
            Raw,
            Root,
            Mod,
        } kind;

        std::map<std::string, NodeId> locals;
        Option<Module::Ptr> boundModule{None};

        /// Define new local.
        /// Returns local node_id that was already defined if it was
        NodeId::Opt define(const ast::Ident::PR & ident);

        /// Searches for name in rib namespace or in bound module (if present)
        /// Returns `false` if failed to resolve a name, or sets resolution in case of success
        bool find(Namespace ns, const std::string & name, NodeId refNodeId, ResStorage & resStorage);

        void bindMod(Module::Ptr module);

        explicit Rib(Kind kind) : kind{kind} {}
    };
}

#endif // JACY_RESOLVE_RIB_H
