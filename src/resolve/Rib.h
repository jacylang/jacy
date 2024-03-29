#ifndef JACY_RESOLVE_RIB_H
#define JACY_RESOLVE_RIB_H

#include <map>

#include "ast/Node.h"
#include "resolve/Module.h"
#include "resolve/Resolutions.h"

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

        /// Maps name to `NodeId` of local variable (`BorrowPat.id`)
        std::map<Symbol, NodeId> locals;
        Option<Module::Ptr> boundModule = None;

        /// Define new local.
        /// Returns local node_id that was already defined if it was
        NodeId::Opt defineLocal(NodeId nodeId, Symbol name);

        NodeId::Opt findLocal(Symbol name);

        void bindMod(Module::Ptr module);

        explicit Rib(Kind kind) : kind{kind} {}
    };
}

#endif // JACY_RESOLVE_RIB_H
