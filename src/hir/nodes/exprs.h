#ifndef JACY_HIR_NODES_EXPRS_H
#define JACY_HIR_NODES_EXPRS_H

#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"
#include "hir/nodes/Pat.h"

namespace jc::hir {
    enum class BinOpKind {
        Add,
        Sub,
        Mul,
        Div,
        Rem,
        Pow,
        And,
        Or,
        Xor,
        BitAnd,
        BitOr,
        Shl,
        Shr,
        Eq,
        LT,
        LE,
        GE,
        GT,
        Spaceship,
        NE,
    };

    enum class PrefixOpKind {
        Not,
        Neg,
        Deref,
    };

    enum class PostfixOpKind {
        Quest,
    };

    using BinOp = span::Spanned<BinOpKind>;
    using PrefixOp = span::Spanned<PrefixOpKind>;
    using PostfixOp = span::Spanned<PostfixOpKind>;

    struct ArrayExpr : Expr {
        ArrayExpr(Expr::List && elements, HirId hirId, Span span)
            : Expr {ExprKind::Array, hirId, span}, elements {std::move(elements)} {
        }

        Expr::List elements;
    };

    struct AssignExpr : Expr {
        AssignExpr(Expr::Ptr && lhs, const parser::Token & op, Expr::Ptr && rhs, HirId hirId, Span span)
            : Expr {ExprKind::Assign, hirId, span}, lhs {std::move(lhs)}, op {op}, rhs {std::move(rhs)} {
        }

        Expr::Ptr lhs;
        parser::Token op;
        Expr::Ptr rhs;
    };

    struct BlockExpr : Expr {
        BlockExpr(Block && block, HirId hirId, Span span)
            : Expr {ExprKind::Block, hirId, span}, block {std::move(block)} {
        }

        Block block;
    };

    struct BorrowExpr : Expr {
        BorrowExpr(bool mut, Expr::Ptr && rhs, HirId hirId, Span span)
            : Expr {ExprKind::Borrow, hirId, span}, mut {mut}, rhs {std::move(rhs)} {
        }

        bool mut;
        Expr::Ptr rhs;
    };

    struct BreakExpr : Expr {
        BreakExpr(Expr::OptPtr && value, HirId hirId, Span span)
            : Expr {ExprKind::Break, hirId, span}, value {std::move(value)} {
        }

        Expr::OptPtr value;
    };

    struct ContinueExpr : Expr {
        ContinueExpr(HirId hirId, Span span) : Expr {ExprKind::Continue, hirId, span} {
        }
    };

    struct DerefExpr : Expr {
        DerefExpr(Expr::OptPtr && rhs, HirId hirId, Span span)
            : Expr {ExprKind::Deref, hirId, span}, rhs {std::move(rhs)} {
        }

        Expr::OptPtr rhs;
    };

    struct FieldExpr : Expr {
        FieldExpr(
            Expr::Ptr && lhs,
            const span::Ident & field,
            HirId hirId,
            Span span
        ) : Expr {ExprKind::Field, hirId, span},
            lhs {std::move(lhs)},
            field {field} {
        }

        Expr::Ptr lhs;
        span::Ident field;
    };

    struct IfExpr : Expr {
        IfExpr(
            Expr::Ptr && cond,
            Block::Opt && ifBranch,
            Block::Opt && elseBranch,
            HirId hirId,
            Span span
        ) : Expr {ExprKind::If, hirId, span},
            cond {std::move(cond)},
            ifBranch {std::move(ifBranch)},
            elseBranch {std::move(elseBranch)} {
        }

        Expr::Ptr cond;
        Block::Opt ifBranch;
        Block::Opt elseBranch;
    };

    struct InfixExpr : Expr {
        InfixExpr(Expr::Ptr && lhs, BinOp op, Expr::Ptr && rhs, HirId hirId, Span span)
            : Expr {ExprKind::Invoke, hirId, span}, lhs {std::move(lhs)}, op {op}, rhs {std::move(rhs)} {
        }

        Expr::Ptr lhs;
        BinOp op;
        Expr::Ptr rhs;
    };

    struct InvokeExpr : Expr {
        InvokeExpr(Expr::Ptr && lhs, Arg::List && args, HirId hirId, Span span)
            : Expr {ExprKind::Invoke, hirId, span}, lhs {std::move(lhs)}, args {std::move(args)} {}

        Expr::Ptr lhs;
        Arg::List args;
    };

    struct LitExpr : Expr {
        using Kind = ast::LitExpr::Kind;
        using ValueT = ast::LitExpr::ValueT;

        LitExpr(Kind kind, ValueT val, HirId hirId, Span span)
            : Expr {ExprKind::Literal, hirId, span}, kind {kind}, val {val} {}

        Kind kind;
        ValueT val;
    };

    struct LoopExpr : Expr {
        LoopExpr(Block && body, HirId hirId, Span span)
            : Expr {ExprKind::Loop, hirId, span}, body {std::move(body)} {
        }

        Block body;
    };

    struct MatchArm : HirNode {
        using List = std::vector<MatchArm>;

        MatchArm(Pat::Ptr && pat, Expr::Ptr && body, HirId hirId, Span span)
            : HirNode {hirId, span},
              pat {std::move(pat)},
              body {std::move(body)} {
        }

        Pat::Ptr pat;
        Expr::Ptr body;
    };

    struct MatchExpr : Expr {
        MatchExpr(Expr::Ptr && expr, MatchArm::List && arms, HirId hirId, Span span)
            : Expr {ExprKind::Match, hirId, span},
              subject {std::move(expr)},
              arms {std::move(arms)} {
        }

        Expr::Ptr subject;
        MatchArm::List arms;
    };

    struct PathExpr : Expr {
        PathExpr(Path && path, HirId hirId, Span span)
            : Expr {ExprKind::Path, hirId, span}, path {std::move(path)} {
        }

        Path path;
    };

    struct PostfixExpr : Expr {
        PostfixExpr(Expr::Ptr && lhs, PostfixOp op, HirId hirId, Span span)
            : Expr {ExprKind::Loop, hirId, span}, lhs {std::move(lhs)}, op {op} {
        }

        Expr::Ptr lhs;
        PostfixOp op;
    };

    struct PrefixExpr : Expr {
        PrefixExpr(PrefixOp op, Expr::Ptr && rhs, HirId hirId, Span span)
            : Expr {ExprKind::Loop, hirId, span}, op {op}, rhs {std::move(rhs)} {
        }

        PrefixOp op;
        Expr::Ptr rhs;
    };

    struct ReturnExpr : Expr {
        ReturnExpr(Expr::OptPtr && value, HirId hirId, Span span)
            : Expr {ExprKind::Return, hirId, span}, value {std::move(value)} {
        }

        Expr::OptPtr value;
    };

    struct TupleExpr : Expr {
        TupleExpr(Expr::List && values, HirId hirId, Span span)
            : Expr {ExprKind::Tuple, hirId, span}, values {std::move(values)} {
        }

        Expr::List values;
    };
}

#endif // JACY_HIR_NODES_EXPRS_H