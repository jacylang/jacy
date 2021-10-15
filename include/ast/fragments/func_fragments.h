#ifndef JACY_AST_FRAGMENTS_FUNC_FRAGMENTS_H
#define JACY_AST_FRAGMENTS_FUNC_FRAGMENTS_H

#include "ast/fragments/Pat.h"
#include "ast/fragments/Type.h"
#include "ast/expr/Expr.h"

namespace jc::ast {
    struct FuncParam : Node {
        using List = std::vector<FuncParam>;

        FuncParam(
            Ident::OptPR && label,
            Pat::Ptr pat,
            Type::Ptr type,
            Expr::OptPtr defaultValue,
            Span span
        ) : Node {span},
            label {std::move(label)},
            pat {std::move(pat)},
            type {std::move(type)},
            defaultValue {std::move(defaultValue)} {
        }

        Ident::OptPR label;
        Pat::Ptr pat;
        Type::Ptr type;
        Expr::OptPtr defaultValue;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct Body {
        Body(bool exprBody, Expr::Ptr && value) : exprBody {exprBody}, value {std::move(value)} {
        }

        bool exprBody;
        Expr::Ptr value;
    };

    struct FuncReturnType {
        using SomeType = Type::Ptr;
        using ValueT = std::variant<Span, SomeType>;

        enum class Kind {
            Default,
            Some,
        };

        FuncReturnType(Span span) : kind {Kind::Default}, val {span} {}
        FuncReturnType(SomeType && type) : kind {Kind::Some}, val {std::move(type)} {}

        auto asDefault() const {
            return std::get<Span>(val);
        }

        const auto & asSome() const {
            return std::get<SomeType>(val);
        }

        auto isDefault() const {
            return kind == Kind::Default;
        }

        auto isSome() const {
            return kind == Kind::Some;
        }

    private:
        Kind kind;
        ValueT val;
    };

    struct FuncSig {
        FuncSig(
            parser::Token::List && modifiers,
            FuncParam::List params,
            FuncReturnType && returnType,
            Span span
        ) : modifiers {modifiers},
            params {std::move(params)},
            returnType {std::move(returnType)},
            span {span} {
        }

        parser::Token::List modifiers;
        FuncParam::List params;
        FuncReturnType returnType;
        Span span;
    };
}

#endif // JACY_AST_FRAGMENTS_FUNC_FRAGMENTS_H
