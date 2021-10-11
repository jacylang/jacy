#ifndef JACY_AST_EXPR_LITEXPR_H
#define JACY_AST_EXPR_LITEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct LitExpr : Expr {
        struct Bool {
            using ValueT = bool;

            ValueT val;
        };

        struct Int {
            using ValueT = uint64_t;

            enum class Kind {
                Unset,

                U8,
                U16,
                U32,
                U64,
                Uint,
                I8,
                I16,
                I32,
                I64,
                Int,
            } kind;
            ValueT val;
        };

        struct Float {
            enum class Kind {
                Unset,

                F32,
                F64,
            };

            /// Symbol of the float value. Not storing actual value as double to avoid any problems,
            /// I'm afraid of problems with float
            span::Symbol sym;
        };

        struct Str {
            span::Symbol sym;
        };

        using ValueT = std::variant<Bool, Int, Float, Str>;

        enum class Kind {
            Bool,
            Int,
            Float,
            Str,
        };

    private:
        LitExpr(ValueT val, const parser::Token & token)
            : Expr {token.span, ExprKind::LiteralConstant}, token {token}, val {val} {}

    public:
        LitExpr fromToken(const parser::Token & tok) {

        }

    public:
        parser::Token token;
        Kind kind;
        ValueT val;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_LITEXPR_H
