#ifndef JACY_SRC_AST_EXPR_EXPRS_H
#define JACY_SRC_AST_EXPR_EXPRS_H

#include "ast/expr/Expr.h"
#include "ast/stmt/Stmt.h"
#include "ast/fragments/Ident.h"
#include "ast/fragments/Pat.h"
#include "ast/fragments/Type.h"
#include <charconv>

namespace jc::ast {
    using NamedExpr = NamedNode<Expr::Ptr, Ident::OptPR>;

    /// `Expr = Expr`
    struct Assign : Expr {
        Assign(Expr::Ptr && lhs, const parser::Token & op, Expr::Ptr && rhs, span::Span span)
            : Expr {span, Expr::Kind::Assign}, lhs {std::move(lhs)}, op {op}, rhs {std::move(rhs)} {}

        Expr::Ptr lhs;
        parser::Token op;
        Expr::Ptr rhs;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `{(STMT;)*}`
    struct Block : Expr {
        using Ptr = PR<N<Block>>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        Block(Stmt::List && stmts, Span span)
            : Expr {span, Expr::Kind::Block},
              stmts {std::move(stmts)} {}

        Stmt::List stmts;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `&mut? EXPR`
    struct BorrowExpr : Expr {
        BorrowExpr(
            bool mut,
            Expr::Ptr expr,
            Span span
        ) : Expr {span, Expr::Kind::Borrow},
            mut {mut},
            expr {std::move(expr)} {}

        bool mut;
        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `break EXPR?;`
    struct BreakExpr : Expr {
        BreakExpr(Expr::OptPtr && expr, Span span)
            : Expr {span, Expr::Kind::Break},
              expr {std::move(expr)} {}

        Expr::OptPtr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `continue`
    struct ContinueExpr : Expr {
        explicit ContinueExpr(Span span) : Expr {span, Expr::Kind::Continue} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `EXPR.EXPR`
    struct FieldExpr : Expr {
        FieldExpr(
            Expr::Ptr && lhs,
            Ident::PR field,
            Span span
        ) : Expr {span, Expr::Kind::Field},
            lhs {std::move(lhs)},
            field {field} {}

        Expr::Ptr lhs;
        Ident::PR field;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// For-Loop
    struct ForExpr : Expr {
        ForExpr(
            Pat::Ptr && pat,
            Expr::Ptr && inExpr,
            Block::Ptr && body,
            Span span
        ) : Expr {span, Expr::Kind::For},
            pat {std::move(pat)},
            inExpr {std::move(inExpr)},
            body {std::move(body)} {}

        Pat::Ptr pat;
        Expr::Ptr inExpr;
        Block::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// If expression
    struct IfExpr : Expr {
        IfExpr(
            Expr::Ptr condition,
            Block::OptPtr ifBranch,
            Block::OptPtr elseBranch,
            Span span
        ) : Expr {span, Expr::Kind::If},
            condition {std::move(condition)},
            ifBranch {std::move(ifBranch)},
            elseBranch {std::move(elseBranch)} {}

        Expr::Ptr condition;
        Block::OptPtr ifBranch;
        Block::OptPtr elseBranch;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `EXPR [INFIX-OP] EXPR`
    struct Infix : Expr {
        Infix(Expr::Ptr lhs, const parser::Token & op, Expr::Ptr rhs, Span span)
            : Expr {span, Expr::Kind::Infix}, lhs {std::move(lhs)}, op {op}, rhs {std::move(rhs)} {}

        Expr::Ptr lhs;
        parser::Token op;
        Expr::Ptr rhs;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `expr '(' ((Identifier ':')? expr (',' (Identifier ':')? expr)* ',')? ')'`
    struct Invoke : Expr {
        using Arg = NamedNode<Expr::Ptr, Ident::OptPR>;

        Invoke(Expr::Ptr lhs, Arg::List args, Span span)
            : Expr {span, Expr::Kind::Invoke}, lhs {std::move(lhs)}, args {std::move(args)} {}

        Expr::Ptr lhs;
        Arg::List args;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct LambdaParam : Node {
        using List = std::vector<LambdaParam>;

        LambdaParam(Pat::Ptr pat, Type::OptPtr type, Span span)
            : Node {span},
              pat {std::move(pat)},
              type {std::move(type)} {}

        Pat::Ptr pat;
        Type::OptPtr type;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct Lambda : Expr {
        Lambda(
            LambdaParam::List params,
            Type::OptPtr returnType,
            Expr::Ptr body,
            Span span
        ) : Expr {span, Expr::Kind::Lambda},
            params {std::move(params)},
            returnType {std::move(returnType)},
            body {std::move(body)} {}

        LambdaParam::List params;
        Type::OptPtr returnType;
        Expr::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ListExpr : Expr {
        ListExpr(Expr::List && elements, Span span)
            : Expr {span, Expr::Kind::List}, elements {std::move(elements)} {}

        Expr::List elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

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

        enum class LitPreEvalErr {
            IntOutOfRange,
        };

    private:
        LitExpr(ValueT val, const parser::Token & token)
            : Expr {token.span, Expr::Kind::LiteralConstant}, token {token}, val {val} {}

        static dt::Result <Int, LitPreEvalErr> intValue(
            parser::TokLit::Kind kind,
            span::Symbol sym,
            span::Symbol::Opt /*TODO: suffix*/
        ) {
            uint8_t base = 0;
            switch (kind) {
                case parser::TokLit::Kind::DecLiteral: {
                    base = 10;
                    break;
                }
                case parser::TokLit::Kind::BinLiteral: {
                    base = 2;
                    break;
                }
                case parser::TokLit::Kind::OctLiteral: {
                    base = 8;
                    break;
                }
                case parser::TokLit::Kind::HexLiteral: {
                    base = 16;
                    break;
                }
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
            auto[ptr, errCode] {
                std::from_chars(strValue.data(), strValue.data() + strValue.size(), value, base)
            };

            if (errCode == std::errc()) {
                return Ok(Int {intKind, value});
            } else if (errCode == std::errc::invalid_argument) {
                log::devPanic("[ast::LitExpr::intValue] Not a number symbol value");
            } else if (errCode == std::errc::result_out_of_range) {
                return Err(LitPreEvalErr::IntOutOfRange);
            }
        }

    public:
        static dt::Result <LitExpr, LitPreEvalErr> fromToken(const parser::Token & tok) {
            if (not tok.isLiteral()) {
                log::devPanic("Called `ast::LitExpr::fromToken` with non-literal token '", tok.dump(), "'");
            }

            const auto & lit = tok.asLit();

            switch (lit.kind) {
                case parser::TokLit::Kind::Bool: {
                    if (lit.sym.isKw(span::Kw::True)) {
                        return Ok(LitExpr {Bool {true}, tok});
                    }
                    if (lit.sym.isKw(span::Kw::False)) {
                        return Ok(LitExpr {Bool {false}, tok});
                    }
                    log::devPanic("[ast::LitExpr::fromToken] Got non-boolean symbol in boolean literal token");
                }

                case parser::TokLit::Kind::FloatLiteral: {
                    return Ok(LitExpr {Float {lit.sym}, tok});
                }

                case parser::TokLit::Kind::SQStringLiteral:
                case parser::TokLit::Kind::DQStringLiteral: {
                    return Ok(LitExpr {Str {lit.sym}, tok});
                }

                case parser::TokLit::Kind::DecLiteral:
                case parser::TokLit::Kind::BinLiteral:
                case parser::TokLit::Kind::OctLiteral:
                case parser::TokLit::Kind::HexLiteral: {
                    auto intVal = intValue(lit.kind, lit.sym, lit.suffix);
                    if (intVal.err()) {
                        return Err(intVal.takeErr());
                    }
                    return Ok(LitExpr {intVal.unwrap(), tok});
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

    struct LoopExpr : Expr {
        LoopExpr(Block::Ptr && body, Span span)
            : Expr {span, Expr::Kind::Loop}, body {std::move(body)} {}

        Block::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct MatchArm : Node {
        using List = std::vector<MatchArm>;

        MatchArm(
            Pat::Ptr pat,
            Expr::Ptr body,
            Span span
        ) : Node {span},
            pat {std::move(pat)},
            body {std::move(body)} {}

        Pat::Ptr pat;
        Expr::Ptr body;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct MatchExpr : Expr {
        MatchExpr(
            Expr::Ptr subject,
            MatchArm::List entries,
            Span span
        ) : Expr {span, Expr::Kind::Match},
            subject {std::move(subject)},
            arms {std::move(entries)} {}

        Expr::Ptr subject;
        MatchArm::List arms;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ParenExpr : Expr {
        ParenExpr(Expr::Ptr && expr, Span span)
            : Expr {span, Expr::Kind::Paren}, expr {std::move(expr)} {}

        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct PathExpr : Expr {
        PathExpr(Path && path) : Expr {path.span, Expr::Kind::Path}, path {std::move(path)} {}

        Path path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Postfix : Expr {
        Postfix(
            Expr::Ptr && lhs,
            const parser::Token & op,
            Span span
        ) : Expr {span, Expr::Kind::Postfix},
            lhs {std::move(lhs)},
            op {op} {}

        Expr::Ptr lhs;
        parser::Token op;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Prefix : Expr {
        Prefix(const parser::Token & op, Expr::Ptr && rhs, Span span)
            : Expr {span, Expr::Kind::Prefix}, op {op}, rhs {std::move(rhs)} {}

        parser::Token op;
        Expr::Ptr rhs;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ReturnExpr : Expr {
        ReturnExpr(Expr::OptPtr && expr, Span span)
            : Expr {span, Expr::Kind::Return},
              expr {std::move(expr)} {}

        Expr::OptPtr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SelfExpr : Expr {
        SelfExpr(Span span) : Expr {span, Expr::Kind::Self} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // Make a fragment and remove from Expr
    struct SpreadExpr : Expr {
        SpreadExpr(const parser::Token & token, Expr::Ptr && expr, Span span)
            : Expr {span, Expr::Kind::Spread}, token {token}, expr {std::move(expr)} {}

        parser::Token token;
        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Subscript : Expr {
        Subscript(Expr::Ptr && lhs, Expr::List && indices, Span span)
            : Expr {span, Expr::Kind::Subscript}, lhs {std::move(lhs)}, indices {std::move(indices)} {}

        Expr::Ptr lhs;
        Expr::List indices;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TupleExpr : Expr {
        using Element = NamedNode<Expr::Ptr, Ident::OptPR>;

        TupleExpr(Element::List && elements, Span span)
            : Expr {span, Expr::Kind::Tuple}, elements {std::move(elements)} {}

        Element::List elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UnitExpr : Expr {
        UnitExpr(Span span) : Expr {span, Expr::Kind::Unit} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct WhileExpr : Expr {
        WhileExpr(
            Expr::Ptr && condition,
            Block::Ptr && body,
            Span span
        ) : Expr {span, Expr::Kind::While},
            condition {std::move(condition)},
            body {std::move(body)} {}

        Expr::Ptr condition;
        Block::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_SRC_AST_EXPR_EXPRS_H
