#ifndef JACY_AST_FRAGMENTS_FUNC_FRAGMENTS_H
#define JACY_AST_FRAGMENTS_FUNC_FRAGMENTS_H

#include "ast/fragments/Pattern.h"
#include "ast/fragments/Type.h"
#include "ast/expr/Expr.h"

namespace jc::ast {
    struct FuncParam : Node {
        using List = std::vector<FuncParam>;

        FuncParam(
            Ident::OptPR && label,
            Pattern::Ptr pat,
            Type::Ptr type,
            Expr::OptPtr defaultValue,
            const Span & span
        ) : Node{span},
            label{std::move(label)},
            pat{std::move(pat)},
            type{std::move(type)},
            defaultValue{std::move(defaultValue)} {}

        Ident::OptPR label;
        Pattern::Ptr pat;
        Type::Ptr type;
        Expr::OptPtr defaultValue;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Body {
        Body(bool exprBody, Expr::Ptr && value) : exprBody{exprBody}, value{std::move(value)} {}

        bool exprBody;
        Expr::Ptr value;
    };

    struct FuncSig {
        FuncSig(
            parser::Token::List && modifiers,
            FuncParam::List params,
            Type::OptPtr returnType
        ) : modifiers{modifiers},
            params{std::move(params)},
            returnType{std::move(returnType)} {}

        parser::Token::List modifiers;
        FuncParam::List params;
        Type::OptPtr returnType;
    };
}

#endif // JACY_AST_FRAGMENTS_FUNC_FRAGMENTS_H
