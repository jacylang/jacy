#ifndef JACY_AST_EXPR_LITEXPR_H
#define JACY_AST_EXPR_LITEXPR_H

#include <charconv>

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct LitExpr : Expr {
        struct Bool {
            using ValueT = bool;

            ValueT val;
        };

        struct Int {
            using ValueT = uint64_t;

            /// Specific kind determined by suffix, e.g. `123i32` is `I32`, `1` is `Unset`
            /// Note: Do not default the kind of int
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

        Int intValue(parser::TokLit::Kind kind, span::Symbol sym, span::Symbol::Opt suffix) {
            uint8_t base = 0;
            switch (kind) {
                case parser::TokLit::Kind::DecLiteral: base = 10; break;
                case parser::TokLit::Kind::BinLiteral: base = 2; break;
                case parser::TokLit::Kind::OctLiteral: base = 8; break;
                case parser::TokLit::Kind::HexLiteral: base = 16; break;
                default: {
                    log::devPanic("Got non-integer literal kind in `ast::LitExpr::intValue`");
                }
            }

            // TODO: Suffixes
            Int::Kind intKind = Int::Kind::Unset;

            std::string strValue;
            if (base != 10) {
                strValue = sym.slice(2);
            } else {
                strValue = sym.toString();
            }

            uint64_t value;
            auto [ptr, errCode] {
                std::from_chars(strValue.data(), strValue.data() + strValue.size(), value, base)
            };

            if (errCode == std::errc()) {
                return Int {intKind, value};
            } else if (errCode == std::errc::invalid_argument) {
                log::devPanic("[ast::LitExpr::intValue] Not a number symbol value");
            } else if (errCode == std::errc::result_out_of_range) {
                // TODO: Error
            }
        }

    public:
        LitExpr fromToken(const parser::Token & tok) {
            if (not tok.isLiteral()) {
                log::devPanic("Called `ast::LitExpr::fromToken` with non-literal token '", tok.dump(), "'");
            }

            const auto & lit = tok.asLit();

            switch (lit.kind) {
                case parser::TokLit::Kind::Bool: {
                    if (lit.sym.isKw(span::Kw::True)) {
                        return LitExpr {Bool {true}, tok};
                    }
                    if (lit.sym.isKw(span::Kw::False)) {
                        return LitExpr {Bool {false}, tok};
                    }
                    log::devPanic("[ast::LitExpr::fromToken] Got non-boolean symbol in boolean literal token");
                }

                case parser::TokLit::Kind::FloatLiteral: {
                    return LitExpr {Float {lit.sym}, tok};
                }

                case parser::TokLit::Kind::SQStringLiteral:
                case parser::TokLit::Kind::DQStringLiteral: {
                    return LitExpr {Str {lit.sym}, tok};
                }

                case parser::TokLit::Kind::DecLiteral:
                case parser::TokLit::Kind::BinLiteral:
                case parser::TokLit::Kind::OctLiteral:
                case parser::TokLit::Kind::HexLiteral: {
                    return LitExpr {intValue(lit.kind, lit.sym, lit.suffix), tok};
                }
            }

            log::devPanic("Unhandled `TokLit::Kind` in `ast::LitExpr::fromToken`");
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
