#ifndef JACY_SRC_AST_ITEM_ITEMS_H
#define JACY_SRC_AST_ITEM_ITEMS_H

#include "ast/item/Item.h"
#include "ast/fragments/Ident.h"
#include "ast/fragments/AnonConst.h"
#include "ast/fragments/Type.h"
#include "ast/fragments/func_fragments.h"
#include "ast/fragments/SimplePath.h"

namespace jc::ast {
    using CommonField = NamedNode<Type::Ptr, Ident::OptPR>;

    struct Variant : Node {
        using List = std::vector<Variant>;

        enum class Kind {
            Unit, // `A = const expr` (optional discriminant)
            Tuple, // `A(a, b, c...)`
            Struct, // `A {a, b, c...}`
        };

        Variant(Ident::PR name, AnonConst::Opt && disc, Span span)
            : Node {span}, kind {Kind::Unit}, name {name}, body {std::move(disc)} {}

        Variant(Kind kind, Ident::PR name, CommonField::List && tupleFields, Span span)
            : Node {span}, kind {kind}, name {name}, body {std::move(tupleFields)} {}

        Kind kind;
        Ident::PR name;
        std::variant<AnonConst::Opt, CommonField::List> body;

        const auto & getDisc() const {
            return std::get<AnonConst::Opt>(body);
        }

        const auto & getFields() const {
            return std::get<CommonField::List>(body);
        }

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct Enum : Item {
        Enum(Ident::PR name, Variant::List && entries, Span span)
            : Item {span, Item::Kind::Enum}, name {name}, variants {std::move(entries)} {}

        Ident::PR name;
        Variant::List variants {};

        span::Ident getName() const override {
            return name.unwrap();
        }

        NodeId::Opt getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Func : Item {
        Func(
            FuncHeader header,
            FuncSig && sig,
            GenericParam::OptList generics,
            Ident::PR name,
            Option<Body> && body,
            Span span
        ) : Item {span, Item::Kind::Func},
            header {std::move(header)},
            sig {std::move(sig)},
            generics {std::move(generics)},
            name {std::move(name)},
            body {std::move(body)} {}

        FuncHeader header;
        FuncSig sig;
        GenericParam::OptList generics;
        Ident::PR name;
        Option<Body> body;

        span::Ident getName() const override {
            return name.unwrap();
        }

        NodeId::Opt getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Impl : Item {
        Impl(
            GenericParam::OptList && generics,
            PR<TypePath::Ptr> && traitTypePath,
            Type::OptPtr && forType,
            Item::List && members,
            Span span
        ) : Item {span, Item::Kind::Impl},
            generics {std::move(generics)},
            traitTypePath {std::move(traitTypePath)},
            forType {std::move(forType)},
            members {std::move(members)} {}

        GenericParam::OptList generics;
        PR<TypePath::Ptr> traitTypePath;
        Type::OptPtr forType;
        Item::List members;

        span::Ident getName() const override {
            return Ident::empty();
        }

        NodeId::Opt getNameNodeId() const override {
            return None;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Init : Item {
        Init(
            FuncSig && sig,
            Option<Body> && body,
            Span span
        ) : Item {span, Item::Kind::Init},
            sig {std::move(sig)},
            body {std::move(body)} {}

        FuncSig sig;
        Option<Body> body;

        span::Ident getName() const override {
            return span::Ident {span::Symbol::intern("init"), span.fromStartWithLen(4)};
        }

        NodeId::Opt getNameNodeId() const override {
            return None;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Mod : Item {
        Mod(
            Ident::PR name,
            Item::List && items,
            Span span
        ) : Item {span, Item::Kind::Mod},
            name {name},
            items {std::move(items)} {}

        Ident::PR name;
        Item::List items;

        span::Ident getName() const override {
            return name.unwrap();
        }

        NodeId::Opt getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Struct : Item {
        Struct(
            Ident::PR name,
            GenericParam::OptList generics,
            StructField::List fields,
            Span span
        ) : Item {span, Item::Kind::Struct},
            name {std::move(name)},
            generics {std::move(generics)},
            fields {std::move(fields)} {}

        Ident::PR name;
        GenericParam::OptList generics;
        StructField::List fields;

        span::Ident getName() const override {
            return name.unwrap();
        }

        NodeId::Opt getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Trait : Item {
        Trait(
            Ident::PR name,
            GenericParam::OptList && generics,
            TypePath::List && superTraits,
            Item::List && members,
            Span span
        ) : Item {span, Item::Kind::Trait},
            name {name},
            generics {std::move(generics)},
            superTraits {std::move(superTraits)},
            members {std::move(members)} {}

        Ident::PR name;
        GenericParam::OptList generics;
        TypePath::List superTraits;
        Item::List members;

        span::Ident getName() const override {
            return name.unwrap();
        }

        NodeId::Opt getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TypeAlias : Item {
        TypeAlias(
            Ident::PR name,
            Type::OptPtr && type,
            Span span
        ) : Item {span, Item::Kind::TypeAlias},
            name {name},
            type {std::move(type)} {}

        Ident::PR name;
        Type::OptPtr type;

        span::Ident getName() const override {
            return name.unwrap();
        }

        NodeId::Opt getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UseTree : Node {
        using PR = PR<UseTree>;
        using List = std::vector<PR>;
        using ValueT = std::variant<std::monostate, Ident::PR, UseTree::List>;

        enum class Kind {
            Raw,
            All,
            Specific,
            Rebind,
        };

        UseTree(SimplePath::Opt && path, span::Span span)
            : Node {span}, kind {Kind::Raw}, path {std::move(path)}, val {std::monostate {}} {}

        UseTree(SimplePath::Opt && path, bool, span::Span span)
            : Node {span}, kind {Kind::All}, path {std::move(path)}, val {std::monostate {}} {}

        UseTree(SimplePath::Opt && path, UseTree::List && specifics, span::Span span)
            : Node {span}, kind {Kind::Specific}, path {std::move(path)}, val {std::move(specifics)} {}

        UseTree(SimplePath::Opt && path, Ident::PR rebinding, span::Span span)
            : Node {span}, kind {Kind::Rebind}, path {std::move(path)}, val {rebinding} {}

        Kind kind;
        SimplePath::Opt path;
        ValueT val;

        const auto & expectPath() const {
            return path.unwrap();
        }

        const auto & expectRebinding() const {
            return std::get<Ident::PR>(val).unwrap();
        }

        const auto & expectSpecifics() const {
            return std::get<UseTree::List>(val);
        }

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct UseDecl : Item {
        UseDecl(
            UseTree::PR && useTree,
            Span span
        ) : Item {span, Item::Kind::Use},
            useTree {std::move(useTree)} {}

        UseTree::PR useTree;

        span::Ident getName() const override {
            return Ident::empty();
        }

        NodeId::Opt getNameNodeId() const override {
            return None;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_SRC_AST_ITEM_ITEMS_H
