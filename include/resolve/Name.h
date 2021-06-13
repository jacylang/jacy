#ifndef JACY_RESOLVE_NAME_H
#define JACY_RESOLVE_NAME_H

#include "ast/Node.h"
#include "resolve/Module.h"

namespace jc::resolve {
    using ast::node_id;
    using ast::opt_node_id;
    
    struct Name;
    struct Rib;
    using rib_ptr = std::shared_ptr<Rib>;
    using opt_rib = dt::Option<rib_ptr>;
    using rib_stack = std::vector<rib_ptr>;
    using name_ptr = std::shared_ptr<Name>;
    using ns_map = std::map<std::string, name_ptr>;

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
                default: {
                    return "[NO REPRESENTATION]";
                }
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
            return "meow, bitch";
        }
    };

    using decl_result = dt::Option<name_ptr>;

    struct Rib {
        enum class Kind {
            Raw,
        } kind;

        // TODO: Use `Ident{node_id, string}` instead of string as key
        ns_map typeNS;
        ns_map valueNS;
        ns_map lifetimeNS;

        /// Declare new name.
        /// Returns kind and node_id of node that was already declared if it was
        decl_result declare(const std::string & name, Name::Kind kind, node_id nodeId);
        decl_result resolve(const std::string & name, Namespace ns);

        ns_map & getNSForName(Name::Kind kind);
        ns_map & getNS(Namespace ns);

        explicit Rib(Kind kind) : kind(kind) {}
    };
}

#endif // JACY_RESOLVE_NAME_H
