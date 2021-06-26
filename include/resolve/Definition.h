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

    struct Def {
        Def(DefKind kind, const span::Span & span) : kind(kind), span(span) {}

        DefKind kind;
        const span::Span span;

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
    };

    struct DefStorage {
        const Def & getDef(def_id defId) const {
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
