#ifndef JACY_TYPEPARAMS_H
#define JACY_TYPEPARAMS_H

#include "ast/expr/Identifier.h"

namespace jc::ast {
    struct Type;
    struct TypeParam;
    using type_param_list = std::vector<std::shared_ptr<TypeParam>>;
    using opt_type_params = dt::Option<type_param_list>;
    using type_ptr = std::shared_ptr<Type>;
    using opt_type_ptr = dt::Option<type_ptr>;

    enum class TypeParamKind {
        Type,
        Lifetime,
        Const,
    };

    struct TypeParam : Node {
        explicit TypeParam(TypeParamKind kind, const Span & span)
            : kind(kind), Node(span) {}

        TypeParamKind kind;

        virtual void accept(BaseVisitor & visitor) = 0;
    };

    struct GenericType : TypeParam {
        GenericType(id_ptr id, opt_type_ptr type, const Span & span)
            : id(std::move(id)), type(std::move(type)), TypeParam(TypeParamKind::Type, span) {}

        id_ptr id;
        opt_type_ptr type;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct Lifetime : TypeParam {
        Lifetime(id_ptr id, const Span & span) : id(std::move(id)), TypeParam(TypeParamKind::Lifetime, span) {}

        id_ptr id;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct ConstParam : TypeParam {
        ConstParam(
            id_ptr id,
            type_ptr type,
            opt_expr_ptr defaultValue,
            const Span & span
        ) : id(std::move(id)),
            type(std::move(type)),
            defaultValue(std::move(defaultValue)),
            TypeParam(TypeParamKind::Const, span) {}

        id_ptr id;
        type_ptr type;
        opt_expr_ptr defaultValue;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_TYPEPARAMS_H


