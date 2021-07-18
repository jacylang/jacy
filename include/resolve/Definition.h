#ifndef JACY_RESOLVE_DEFINITION_H
#define JACY_RESOLVE_DEFINITION_H

#include "span/Span.h"
#include "ast/Node.h"

namespace jc::resolve {
    struct Module;
    struct DefId;
    using module_ptr = std::shared_ptr<Module>;
    using opt_module_ptr = Option<module_ptr>;
    using opt_def_id = Option<DefId>;
    using def_depth = uint32_t;
    using DefIndex = size_t;

    const DefIndex ROOT_DEF_INDEX = 0;

    struct DefId {
        explicit DefId(DefIndex index) : index(index) {}

        DefIndex getIndex() const {
            return index;
        }

    private:
        DefIndex index;
    };

    const DefId ROOT_DEF_ID = DefId(ROOT_DEF_INDEX);

    inline std::ostream & operator<<(std::ostream & os, const DefId & defId) {
        std::cout << defId.getIndex();
        return os;
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

    struct Def {
        Def(
            def_depth depth,
            DefVis vis,
            DefKind kind,
            const Option<span::Span> & nameSpan,
            ast::opt_node_id nameNodeId
        ) : depth(depth),
            vis(vis),
            kind(kind),
            nameNodeId(nameNodeId),
            nameSpan(nameSpan) {}

        def_depth depth;
        DefVis vis;
        DefKind kind;
        const ast::opt_node_id nameNodeId;
        const Option<span::Span> nameSpan;

        static inline Namespace getNS(DefKind kind) {
            switch (kind) {
                case DefKind::Enum:
                case DefKind::Mod:
                case DefKind::Trait:
                case DefKind::TypeAlias:
                case DefKind::TypeParam:
                case DefKind::Dir:
                case DefKind::File:
                case DefKind::Struct:
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
                default:;
            }

            common::Logger::notImplemented("Definition::getNS");
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
                case DefKind::Dir:
                    return "[DIR]";
                case DefKind::File:
                    return "[FILE]";
                case DefKind::Root:
                    return "[ROOT]";
                case DefKind::Impl:
                    return "`impl`";
                case DefKind::Mod:
                    return "`mod`";
                case DefKind::Variant:
                    return "`enum` variant";
                default: return "[NO REPRESENTATION (bug)]";
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

            common::Logger::notImplemented("Definition::isUsableAs");
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

        // Debug //
        friend std::ostream & operator<<(std::ostream & os, const Def & def) {
            os << def.kindStr();
            if (def.nameNodeId.some()) {
                os << " [nameNode: #" << def.nameNodeId.unwrap() << "]";
            }
            return os;
        }
    };
}

#endif // JACY_RESOLVE_DEFINITION_H
