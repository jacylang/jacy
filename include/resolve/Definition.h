#ifndef JACY_RESOLVE_DEFINITION_H
#define JACY_RESOLVE_DEFINITION_H

#include "span/Span.h"
#include "ast/Node.h"

namespace jc::resolve {
    struct Module;
    using module_ptr = std::shared_ptr<Module>;
    using def_id = size_t;

    enum class Namespace {
        Value,
        Type,
        Lifetime,
    };

    enum class DefKind {
        Dir,
        File,
        Root,

        Const,
        ConstParam,
        Enum,
        Func,
        Impl,
        Lifetime,
        Mod,
        Struct,
        Trait,
        TypeAlias,
        TypeParam,
        Variant,
    };

    enum class NameUsage {
        Type,
        Expr,
        Lifetime,
    };

    struct Def {
        Def(
            DefKind kind,
            const dt::Option<span::Span> & nameSpan,
            ast::opt_node_id nameNodeId
        ) : kind(kind),
            nameNodeId(nameNodeId),
            nameSpan(nameSpan) {}

        DefKind kind;
        const ast::opt_node_id nameNodeId;
        const dt::Option<span::Span> nameSpan;

        static inline constexpr Namespace getNS(DefKind kind) {
            switch (kind) {
                case DefKind::Enum:
                case DefKind::Mod:
                case DefKind::Trait:
                case DefKind::TypeAlias:
                case DefKind::TypeParam:
                case DefKind::Variant: {
                    return Namespace::Type;
                }
                case DefKind::Const: {
                case DefKind::ConstParam:
                case DefKind::Func:
                    return Namespace::Value;
                }
                case DefKind::Lifetime: {
                    return Namespace::Lifetime;
                }
                default: {
                    common::Logger::devPanic("Called `Def::getNS` with non-namespace `DefKind`");
                }
            }
        }

        static std::string kindStr(DefKind kind) {
            switch (kind) {
                case DefKind::Const:
                    return "`const`";
                case DefKind::Struct:
                    return "`struct`";
                case DefKind::Trait:
                    return "`trait`";
                case DefKind::TypeParam:
                    return "type parameter";
                case DefKind::Lifetime:
                    return "lifetime parameter";
                case DefKind::ConstParam:
                    return "`const` parameter";
                case DefKind::Func:
                    return "`func`";
                case DefKind::Enum:
                    return "`enum`";
                case DefKind::TypeAlias:
                    return "`type` alias";
                default: {
                    return "[NO REPRESENTATION]";
                }
            }
        }

        std::string kindStr() const {
            return kindStr(kind);
        }

        static bool isUsableAs(DefKind kind, NameUsage usage) {
            if (usage == NameUsage::Lifetime) {
                switch (kind) {
                    case DefKind::Lifetime:
                        return true;
                    default:
                        return false;
                }
            }
            if (usage == NameUsage::Type) {
                switch (kind) {
                    case DefKind::Struct:
                    case DefKind::Trait:
                    case DefKind::TypeAlias:
                    case DefKind::TypeParam: {
                        return true;
                    }
                    default: return false;
                }
            }
            if (usage == NameUsage::Expr) {
                switch (kind) {
                    case DefKind::Const:
                    case DefKind::ConstParam:
                    case DefKind::Func: {
                        return true;
                    }
                    default: return false;
                }
            }
            return false;
        }

        bool isUsableAs(NameUsage usage) const {
            return isUsableAs(kind, usage);
        }

        static std::string usageToString(NameUsage usage) {
            switch (usage) {
                case NameUsage::Type: return "type";
                case NameUsage::Expr: return "expression";
                case NameUsage::Lifetime: return "lifetime";
            }
        }

        // Debug //
        friend std::ostream & operator<<(std::ostream & os, const Def & def) {
            os << def.kindStr();
            if (def.nameNodeId) {
                os << " [" << def.nameNodeId.unwrap() << "]";
            }
            return os;
        }
    };

    struct DefStorage {
        const Def & getDef(def_id defId) const {
            if (defId >= defs.size()) {
                common::Logger::devPanic("Called `DefStorage::getDef` with non-existent `defId`");
            }
            return defs.at(defId);
        }

        template<class ...Args>
        def_id define(Args ...args) {
            defs.emplace_back(Def(args...));
            return defs.size() - 1;
        }

    private:
        std::vector<Def> defs;
    };
}

#endif // JACY_RESOLVE_DEFINITION_H
