#ifndef JACY_RESOLVE_DEFINITION_H
#define JACY_RESOLVE_DEFINITION_H

#include "span/Span.h"
#include "span/Ident.h"
#include "ast/Node.h"

namespace jc::resolve {
    struct FuncName {
        span::Ident baseName;
        span::Symbol suffix;
    };

    struct DefIndex {
        DefIndex(size_t val) : val{val} {}

        size_t val;

        static const DefIndex ROOT_INDEX;

        constexpr bool operator==(const DefIndex & other) const {
            return val == other.val;
        }

        constexpr bool operator<(const DefIndex & other) const {
            return val < other.val;
        }

        auto isRoot() const {
            return val == ROOT_INDEX.val;
        }

        auto toString() const {
            return std::to_string(val);
        }

        friend std::ostream & operator<<(std::ostream & os, const DefIndex & defIndex) {
            return os << log::Color::Green << "#" << defIndex.val << log::Color::Reset;
        }
    };

    struct DefId {
        using Opt = Option<DefId>;

        static const DefId ROOT_DEF_ID;

        explicit DefId(DefIndex index) : index{index} {}

        const DefIndex & getIndex() const {
            return index;
        }

        constexpr bool operator==(const DefId & other) const {
            return other.index == index;
        }

        constexpr bool operator<(const DefId & other) const {
            return index < other.index;
        }

    private:
        DefIndex index;
    };

    inline std::ostream & operator<<(std::ostream & os, const DefId & defId) {
        return os << defId.getIndex();
    }

    enum class Namespace {
        Value,
        Type,
        Lifetime,
    };

    enum class DefVis {
        Unset,
        Pub,
    };

    enum class DefKind {
        Const,
        ConstParam,
        Enum,
        Func,
        Impl,
        Init,
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
        Value,
        Lifetime,
    };

    // Stolen from Rust's source code
    // As, actually, mostly everything else ❤🔥
    template<class T>
    struct PerNS {
        T value;
        T type;
        T lifetime;

        const T & get(Namespace ns) const {
            switch (ns) {
                case Namespace::Value: return value;
                case Namespace::Type: return type;
                case Namespace::Lifetime: return lifetime;
            }
        }

        void set(Namespace ns, const T & t) {
            switch (ns) {
                case Namespace::Value: value = t; break;
                case Namespace::Type: type = t; break;
                case Namespace::Lifetime: lifetime = t; break;
            }
        }

        void each(const std::function<void(const T & ns, Namespace)> & cb) const {
            cb(get(Namespace::Value), Namespace::Value);
            cb(get(Namespace::Type), Namespace::Type);
            cb(get(Namespace::Lifetime), Namespace::Lifetime);
        }

        static void eachKind(const std::function<void(Namespace)> & cb) {
            cb(Namespace::Value);
            cb(Namespace::Type);
            cb(Namespace::Lifetime);
        }
    };

    struct BaseDef {
        BaseDef(
            DefId defId,
            DefKind kind
        ) : defId{defId},
            kind{kind} {}

        DefId defId;
        DefKind kind;

        static inline span::Ident getFuncIdent(const span::Ident & baseName, span::Symbol suffix) {
            return span::Ident {baseName.sym + suffix, baseName.span};
        }

        static inline Namespace getNS(DefKind kind) {
            switch (kind) {
                case DefKind::Enum:
                case DefKind::Mod:
                case DefKind::Trait:
                case DefKind::TypeAlias:
                case DefKind::TypeParam:
                case DefKind::Struct:
                case DefKind::Impl:
                case DefKind::Variant: {
                    return Namespace::Type;
                }
                case DefKind::Const: {
                case DefKind::ConstParam:
                case DefKind::Func:
                case DefKind::Init:
                    return Namespace::Value;
                }
                case DefKind::Lifetime: {
                    return Namespace::Lifetime;
                }
            }

            log::notImplemented("Definition::getNS");
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
                case DefKind::Impl:
                    return "`impl` (implementation)";
                case DefKind::Mod:
                    return "`mod`";
                case DefKind::Variant:
                    return "`enum` variant";
                case DefKind::Init:
                    return "`init` (initializer)";
            }

            return "[NO REPRESENTATION (bug)]";
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

            if (usage == NameUsage::Value) {
                switch (kind) {
                    case DefKind::Const:
                    case DefKind::ConstParam:
                    case DefKind::Func: {
                        return true;
                    }
                    default: return false;
                }
            }

            log::notImplemented("Definition::isUsableAs");
        }

        bool isUsableAs(NameUsage usage) const {
            return isUsableAs(kind, usage);
        }

        static std::string usageToString(NameUsage usage) {
            switch (usage) {
                case NameUsage::Type: return "type";
                case NameUsage::Value: return "value";
                case NameUsage::Lifetime: return "lifetime";
                default: return "[NO REPRESENTATION (bug)]";
            }
        }

        static std::string nsAsUsageStr(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return "expression";
                case Namespace::Type: return "type";
                case Namespace::Lifetime: return "lifetime";
            }
        }
    };

    /// Resolved definition
    struct DefRes : BaseDef {
        using BaseDef::BaseDef;
    };

    struct Def : BaseDef {
        Def(DefId defId, DefKind kind, span::Ident ident) : BaseDef{defId, kind}, ident{ident} {}

        span::Ident ident;
    };

    inline std::ostream & operator<<(std::ostream & os, const Def & def) {
        return os << def.kindStr() << " '" << def.ident << "'" << def.defId;
    }
}

#endif // JACY_RESOLVE_DEFINITION_H
