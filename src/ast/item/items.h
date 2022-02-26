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

        const CommonField::List & getFields() const {
            return std::get<CommonField::List>(body);
        }

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct Enum : ItemKind {
        Enum(Ident::PR && name, Variant::List && entries)
            : ItemKind {ItemKind::Kind::Enum},
              name {std::move(name)},
              variants {std::move(entries)} {}

        Ident::PR name;
        Variant::List variants;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Func : ItemKind {
        Func(
            FuncHeader header,
            Ident::PR && name,
            FuncSig && sig,
            GenericParam::OptList generics,
            Option<Body> && body
        ) : ItemKind {ItemKind::Kind::Func},
            header {std::move(header)},
            name {std::move(name)},
            sig {std::move(sig)},
            generics {std::move(generics)},
            body {std::move(body)} {}

        FuncHeader header;
        Ident::PR name;
        FuncSig sig;
        GenericParam::OptList generics;
        Option<Body> body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Impl : ItemKind {
        Impl(
            GenericParam::OptList && generics,
            PR<TypePath::Ptr> && traitTypePath,
            Type::OptPtr && forType,
            Item::List && members
        ) : ItemKind {ItemKind::Kind::Impl},
            generics {std::move(generics)},
            traitTypePath {std::move(traitTypePath)},
            forType {std::move(forType)},
            members {std::move(members)} {}

        GenericParam::OptList generics;
        PR<TypePath::Ptr> traitTypePath;
        Type::OptPtr forType;
        Item::List members;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Init : ItemKind {
        Init(
            FuncSig && sig,
            Option<Body> && body
        ) : ItemKind {ItemKind::Kind::Init},
            sig {std::move(sig)},
            body {std::move(body)} {}

        FuncSig sig;
        Option<Body> body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Mod : ItemKind {
        Mod(
            Ident::PR && name,
            Item::List && items
        ) : ItemKind {ItemKind::Kind::Mod},
            name {std::move(name)},
            items {std::move(items)} {}

        Ident::PR name;
        Item::List items;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Struct : ItemKind {
        Struct(
            Ident::PR && name,
            GenericParam::OptList generics,
            CommonField::List fields
        ) : ItemKind {ItemKind::Kind::Struct},
            name {std::move(name)},
            generics {std::move(generics)},
            fields {std::move(fields)} {}

        Ident::PR name;
        GenericParam::OptList generics;
        CommonField::List fields;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Trait : ItemKind {
        Trait(
            Ident::PR && name,
            GenericParam::OptList && generics,
            TypePath::List && superTraits,
            Item::List && members
        ) : ItemKind {ItemKind::Kind::Trait},
            name {std::move(name)},
            generics {std::move(generics)},
            superTraits {std::move(superTraits)},
            members {std::move(members)} {}

        Ident::PR name;
        GenericParam::OptList generics;
        TypePath::List superTraits;
        Item::List members;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TypeAlias : ItemKind {
        TypeAlias(
            Ident::PR && name,
            Type::OptPtr && type
        ) : ItemKind {ItemKind::Kind::TypeAlias},
            name {std::move(name)},
            type {std::move(type)} {}

        Ident::PR name;
        Type::OptPtr type;

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

    struct UseDecl : ItemKind {
        UseDecl(
            UseTree::PR && useTree
        ) : ItemKind {ItemKind::Kind::Use},
            useTree {std::move(useTree)} {}

        UseTree::PR useTree;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_SRC_AST_ITEM_ITEMS_H
