#ifndef JACY_RESOLVE_NAME_H
#define JACY_RESOLVE_NAME_H

#include "ast/Node.h"

namespace jc::resolve {
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
            Field,
            Param,
        } kind;

        enum class Usage {
            Type,
            Expr,
            Lifetime,
        };

        ast::node_id nodeId;

        Name(Kind kind, ast::node_id nodeId) : kind(kind), nodeId(nodeId) {}

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
                case Kind::Field:
                    return "field";
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
                    case Kind::Field:
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

    using decl_result = dt::Option<std::tuple<Name::Kind, ast::node_id>>;

    // FIXME: Add rib kinds
    struct Rib {
        std::map<std::string, name_ptr> names;

        /// Declare new name.
        /// Returns node_id of node that was already declared if it was
        decl_result declare(const std::string & name, Name::Kind kind, ast::node_id nodeId);
    };
}

#endif // JACY_RESOLVE_NAME_H
