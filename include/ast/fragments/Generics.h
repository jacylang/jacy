#ifndef JACY_AST_FRAGMENTS_GENERICS_H
#define JACY_AST_FRAGMENTS_GENERICS_H

#include "ast/fragments/Ident.h"

namespace jc::ast {
    struct Type;
    struct GenericParam;
    using gen_param_list = std::vector<N<GenericParam>>;
    using opt_gen_params = Option<gen_param_list>;
    using type_ptr = PR<N<Type>>;
    using opt_type_ptr = Option<type_ptr>;

    enum class GenericParamKind {
        Type,
        Lifetime,
        Const,
    };

    struct GenericParam : Node {
        explicit GenericParam(GenericParamKind kind, const Span & span) : Node(span), kind(kind) {}
        virtual ~GenericParam() = default;

        GenericParamKind kind;

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct TypeParam : GenericParam {
        TypeParam(
            ident_pr name,
            opt_type_ptr type,
            const Span & span
        ) : GenericParam(GenericParamKind::Type, span),
            name(std::move(name)),
            boundType(std::move(type)) {}

        ident_pr name;
        opt_type_ptr boundType;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Lifetime : GenericParam {
        Lifetime(ident_pr name, const Span & span)
            : GenericParam(GenericParamKind::Lifetime, span),
              name(std::move(name)) {}

        ident_pr name;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ConstParam : GenericParam {
        ConstParam(
            ident_pr name,
            type_ptr type,
            Expr::OptPtr defaultValue,
            const Span & span
        ) : GenericParam(GenericParamKind::Const, span),
            name(std::move(name)),
            type(std::move(type)),
            defaultValue(std::move(defaultValue)) {}

        ident_pr name;
        type_ptr type;
        Expr::OptPtr defaultValue;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_GENERICS_H


