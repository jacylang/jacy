#ifndef JACY_RESOLVE_NAME_H
#define JACY_RESOLVE_NAME_H

#include "ast/Node.h"

namespace jc::resolve {
    using ast::node_id;
    using ast::opt_node_id;
    
    struct Name;
    struct Rib;
    using rib_ptr = std::shared_ptr<Rib>;
    using opt_rib = dt::Option<rib_ptr>;
    using rib_stack = std::vector<rib_ptr>;
    using name_ptr = std::shared_ptr<Name>;
    
    struct Name {
        enum class Kind {
            Const,
            Struct,
            Trait,
            Local,
            TypeParam,
            Lifetime,
            ConstParam,
            Func,
            Enum,
            TypeAlias,
            Param,
        } kind;

        enum class Usage {
            Type,
            Expr,
            Lifetime,
        };

        node_id nodeId; // Syntax-unit nodeId

        Name(Kind kind, node_id nodeId) : kind(kind), nodeId(nodeId) {}

        static std::string kindStr(Kind kind) {
            switch (kind) {
                case Kind::Const:
                    return "`const`";
                case Kind::Struct:
                    return "`struct`";
                case Kind::Trait:
                    return "`trait`";
                case Kind::Local:
                    return "local variable";
                case Kind::TypeParam:
                    return "type parameter";
                case Kind::Lifetime:
                    return "lifetime parameter";
                case Kind::ConstParam:
                    return "`const` parameter";
                case Kind::Func:
                    return "`func`";
                case Kind::Enum:
                    return "`enum`";
                case Kind::TypeAlias:
                    return "`type` alias";
                case Kind::Param:
                    return "`func` parameter";
            }
        }

        std::string kindStr() const {
            return kindStr(kind);
        }

        static bool isUsableAs(Kind kind, Usage usage) {
            if (usage == Usage::Lifetime) {
                switch (kind) {
                    case Kind::Lifetime:
                        return true;
                    default:
                        return false;
                }
            }
            if (usage == Usage::Type) {
                switch (kind) {
                    case Kind::Struct:
                    case Kind::Trait:
                    case Kind::TypeAlias:
                    case Kind::TypeParam: {
                        return true;
                    }
                    default: return false;
                }
            }
            if (usage == Usage::Expr) {
                switch (kind) {
                    case Kind::Const:
                    case Kind::Param:
                    case Kind::Local:
                    case Kind::ConstParam:
                    case Kind::Func: {
                        return true;
                    }
                    default: return false;
                }
            }
            return false;
        }

        bool isUsableAs(Usage usage) const {
            return isUsableAs(kind, usage);
        }

        static std::string usageToString(Usage usage) {
            switch (usage) {
                case Usage::Type: return "type";
                case Usage::Expr: return "expression";
                case Usage::Lifetime: return "lifetime";
            }
        }
    };

    using decl_result = dt::Option<std::tuple<Name::Kind, node_id>>;

    struct Rib {
        enum class Kind {
            Normal,
            Item,
            Struct,
            Mod,
        } kind;

        std::map<std::string, name_ptr> names;

        /// Declare new name.
        /// Returns kind and node_id of node that was already declared if it was
        decl_result declare(const std::string & name, Name::Kind kind, node_id nodeId);

        explicit Rib(Kind kind) : kind(kind) {}
    };

    struct ModRib : Rib {
        ModRib(const std::string & name, const span::Span & span)
            : name(name), span(span), Rib(Kind::Mod) {}

        std::string name;
        span::Span span;
    };

    struct ItemRib : Rib {
        explicit ItemRib(node_id nameNodeId) : nameNodeId(nameNodeId), Rib(Kind::Item) {}

        node_id nameNodeId;
    };

    struct StructRib : Rib {
        explicit StructRib(node_id nameNodeId) : nameNodeId(nameNodeId), Rib(Kind::Struct) {}

        node_id nameNodeId;
        std::map<std::string, node_id> fields; // {fieldName: Field node}
    };
}

#endif // JACY_RESOLVE_NAME_H
