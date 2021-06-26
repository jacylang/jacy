#ifndef JACY_RESOLVE_NAME_H
#define JACY_RESOLVE_NAME_H

#include <map>

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
    using ns_map = std::map<std::string, Name>;
    using opt_name = dt::Option<Name>;

    enum class RibNamespace {
        Value,
        Type,
        Lifetime,
    };

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

        const static std::map<Kind, const std::string> kindsStrings;

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

        // Debug //
        friend std::ostream & operator<<(std::ostream & os, const Name & name) {
            return os << name.kindsStrings.at(name.kind) << ":" << name.nodeId;
        }
    };

    struct Rib {
        enum class Kind {
            Raw,
            Root,
            Mod,
        } kind;

        // TODO: Maybe use `Ident{node_id, string}` instead of string as key, to disambiguate
        ns_map typeNS;
        ns_map valueNS;
        ns_map lifetimeNS;
        dt::Option<module_ptr> boundModule{dt::None};

        /// Define new name.
        /// Returns `Name` that was already defined if it was
        opt_name define(const std::string & name, Name::Kind kind, node_id nodeId);

        /// Resolves name in rib namespace
        /// Returns `None` if no `Name` found
        opt_name resolve(const std::string & name, RibNamespace nsKind);

        ns_map & getNSForName(Name::Kind kind);
        ns_map & getNS(RibNamespace nsKind);

        void bindMod(module_ptr module);

        explicit Rib(Kind kind) : kind(kind) {}
    };
}

#endif // JACY_RESOLVE_NAME_H
