#ifndef JACY_AST_ITEM_ITEM_H
#define JACY_AST_ITEM_ITEM_H

#include "ast/fragments/Attr.h"

namespace jc::ast {
    enum class VisKind {
        Unset,
        Pub,
    };

    /// Visibility (`pub` or unset)
    // TODO: Don't use optional span, try to figure out where user would place `pub` or not to make suggestions better.
    struct Vis {
        Vis() : kind {VisKind::Unset}, span {None} {}

        Vis(VisKind kind, const span::Span::Opt & span) : kind {kind}, span {span} {}

        VisKind kind;
        span::Span::Opt span;
    };

    struct Item : Node {
        using Ptr = PR<N<Item>>;
        using List = std::vector<Ptr>;

        enum class Kind {
            Const,
            Enum,
            Func,
            Impl,
            Init,
            Mod,
            Struct,
            Trait,
            TypeAlias,
            Use,
        };

        Item(Span span, Kind kind) : Node {span}, kind {kind} {}

        Attr::List attributes;
        Kind kind;
        Vis vis;

        void setAttributes(Attr::List && attributes) {
            this->attributes = std::move(attributes);
        }

        void setVis(Vis && vis) {
            this->vis = std::move(vis);
        }

        virtual span::Ident getName() const = 0;

        virtual NodeId::Opt getNameNodeId() const = 0;

        template<class T>
        static T * as(const N<Item> & item) {
            return static_cast<T*>(item.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;

        static std::string kindStr(Kind kind) {
            switch (kind) {
                case Kind::Enum: {
                    return "enumeration";
                }
                case Kind::Func: {
                    return "function";
                }
                case Kind::Impl: {
                    return "implementation";
                }
                case Kind::Init: {
                    return "initializer";
                }
                case Kind::Mod: {
                    return "module";
                }
                case Kind::Struct: {
                    return "structure";
                }
                case Kind::Trait: {
                    return "trait";
                }
                case Kind::TypeAlias:{
                    return "type alias";
                }
                case Kind::Use: {
                    return "use declaration";
                }
            }
        }
    };
}

#endif // JACY_AST_ITEM_ITEM_H
