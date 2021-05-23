#ifndef JACY_RESOLVE_NAME_H
#define JACY_RESOLVE_NAME_H

#include "ast/Node.h"

namespace jc::resolve {
    struct Name;
    struct Rib;
    using rib_ptr = std::shared_ptr<Rib>;
    using opt_node_id = dt::Option<ast::node_id>;

    template<class T>
    using name_map = std::map<std::string, std::shared_ptr<T>>;

    struct Name {
        enum class Kind {
            Const,
            Struct,
            Trait,
            Local,
            TypeParam,
            Lifetime,
            ConstParam,
        } kind;
        ast::node_id nodeId;

        Name(Kind kind, ast::node_id nodeId) : kind(kind), nodeId(nodeId) {}

        static std::string kindStr(Kind kind) {
            switch (kind) {
                case Kind::Const: return "`const`";
                case Kind::Struct: return "`struct`";
                case Kind::Trait: return "`trait`";
                case Kind::Local: return "local variable";
                case Kind::TypeParam: return "type parameter";
                case Kind::Lifetime: return "lifetime parameter";
                case Kind::ConstParam: return "`const` parameter";
            }
        }

        std::string kindStr() const {
            return kindStr(kind);
        }
    };

    using decl_result = dt::Option<std::tuple<Name::Kind, ast::node_id>>;

    // FIXME: Add rib kinds
    struct Rib {
        Rib() : parent(dt::None) {}
        Rib(rib_ptr parent) : parent(parent) {}

        dt::Option<rib_ptr> parent;
        name_map<Name> names;

        /// Declare new name.
        /// Returns node_id of node that was already declared if it was
        decl_result declare(const std::string & name, Name::Kind kind, ast::node_id nodeId);
    };
}

#endif // JACY_RESOLVE_NAME_H
