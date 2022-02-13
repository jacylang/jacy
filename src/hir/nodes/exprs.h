#ifndef JACY_HIR_NODES_EXPRS_H
#define JACY_HIR_NODES_EXPRS_H

#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"
#include "hir/nodes/Pat.h"
#include "ast/expr/exprs.h"

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

    static inline std::string binOpStr(BinOpKind binOp) {
        switch (binOp) {
            case BinOpKind::Add:
                return "+";
            case BinOpKind::Sub:
                return "-";
            case BinOpKind::Mul:
                return "*";
            case BinOpKind::Div:
                return "/";
            case BinOpKind::Rem:
                return "%";
            case BinOpKind::Pow:
                return "**";
            case BinOpKind::And:
                return "and";
            case BinOpKind::Or:
                return "or";
            case BinOpKind::Xor:
                return "^";
            case BinOpKind::BitAnd:
                return "&";
            case BinOpKind::BitOr:
                return "|";
            case BinOpKind::Shl:
                return "<<";
            case BinOpKind::Shr:
                return ">>";
            case BinOpKind::Eq:
                return "==";
            case BinOpKind::LT:
                return "<";
            case BinOpKind::LE:
                return "<=";
            case BinOpKind::GE:
                return ">=";
            case BinOpKind::GT:
                return ">";
            case BinOpKind::Spaceship:
                return "<=>";
            case BinOpKind::NE:
                return "!=";
        }
    }

    enum class PrefixOpKind {
        Not,
        Neg,
        Deref,
    };

    static inline std::string prefixOpStr(PrefixOpKind prefixOp) {
        switch (prefixOp) {
            case PrefixOpKind::Not:
                return "not";
            case PrefixOpKind::Neg:
                return "-";
            case PrefixOpKind::Deref:
                return "*";
        }
    }

    enum class PostfixOpKind {
        Quest,
    };

    static inline std::string postfixOpKind(PostfixOpKind postfixOp) {
        switch (postfixOp) {
            case PostfixOpKind::Quest:
                return "?";
        }
    }

    using BinOp = span::Spanned<BinOpKind>;
    using PrefixOp = span::Spanned<PrefixOpKind>;
    using PostfixOp = span::Spanned<PostfixOpKind>;

    struct ArrayExpr : ExprKind {
        ArrayExpr(ExprKind::List && elements)
            : ExprKind {ExprKind::Kind::Array}, elements {std::move(elements)} {}

        ExprKind::List elements;
    };

    struct AssignExpr : ExprKind {
        AssignExpr(Expr && lhs, const parser::Token & op, Expr && rhs)
            : ExprKind {ExprKind::Kind::Assign}, lhs {std::move(lhs)}, op {op}, rhs {std::move(rhs)} {}

        Expr lhs;
        parser::Token op;
        Expr rhs;
    };

    struct BlockExpr : ExprKind {
        BlockExpr(Block && block)
            : ExprKind {ExprKind::Kind::Block}, block {std::move(block)} {}

        Block block;
    };

    struct BorrowExpr : ExprKind {
        BorrowExpr(bool mut, Expr && rhs)
            : ExprKind {ExprKind::Kind::Borrow}, mut {mut}, rhs {std::move(rhs)} {}

        bool mut;
        Expr rhs;
    };

    struct BreakExpr : ExprKind {
        BreakExpr(Expr::Opt && value) : ExprKind {ExprKind::Kind::Break}, value {std::move(value)} {}

        Expr::Opt value;
    };

    struct ContinueExpr : ExprKind {
        ContinueExpr() : ExprKind {ExprKind::Kind::Continue} {}
    };

    struct DerefExpr : ExprKind {
        DerefExpr(Expr::Opt && rhs) : ExprKind {ExprKind::Kind::Deref}, rhs {std::move(rhs)} {}

        Expr::Opt rhs;
    };

    struct FieldExpr : ExprKind {
        FieldExpr(
            Expr && lhs,
            const span::Ident & field
        ) : ExprKind {ExprKind::Kind::Field},
            lhs {std::move(lhs)},
            field {field} {}

        Expr lhs;
        span::Ident field;
    };

    struct IfExpr : ExprKind {
        IfExpr(
            Expr && cond,
            Block::Opt && ifBranch,
            Block::Opt && elseBranch
        ) : ExprKind {ExprKind::Kind::If},
            cond {std::move(cond)},
            ifBranch {std::move(ifBranch)},
            elseBranch {std::move(elseBranch)} {}

        Expr cond;
        Block::Opt ifBranch;
        Block::Opt elseBranch;
    };

    struct InfixExpr : ExprKind {
        InfixExpr(Expr && lhs, BinOp op, Expr && rhs)
            : ExprKind {ExprKind::Kind::Invoke}, lhs {std::move(lhs)}, op {op}, rhs {std::move(rhs)} {}

        Expr lhs;
        BinOp op;
        Expr rhs;
    };

    struct InvokeExpr : ExprKind {
        InvokeExpr(Expr && lhs, Arg::List && args)
            : ExprKind {ExprKind::Kind::Invoke}, lhs {std::move(lhs)}, args {std::move(args)} {}

        Expr lhs;
        Arg::List args;
    };

    struct LitExpr : ExprKind {
        using Kind = ast::LitExpr::Kind;
        using ValueT = ast::LitExpr::ValueT;

        LitExpr(Kind kind, ValueT val, parser::Token token)
            : ExprKind {ExprKind::Kind::Literal}, kind {kind}, val {val}, token {token} {}

        Kind kind;
        ValueT val;

        // TODO: Maybe remove token and add to-token-conversion API
        parser::Token token;
    };

    struct LoopExpr : ExprKind {
        LoopExpr(Block && body)
            : ExprKind {ExprKind::Kind::Loop}, body {std::move(body)} {}

        Block body;
    };

    struct MatchArm {
        using List = std::vector<MatchArm>;

        MatchArm(Pat::Ptr && pat, Expr && body, HirId hirId, Span span)
            : hirId {hirId},
              span {span},
              pat {std::move(pat)},
              body {std::move(body)} {}

        HirId hirId;
        Span span;
        Pat::Ptr pat;
        Expr body;
    };

    struct MatchExpr : ExprKind {
        MatchExpr(Expr && expr, MatchArm::List && arms)
            : ExprKind {ExprKind::Kind::Match},
              subject {std::move(expr)},
              arms {std::move(arms)} {}

        Expr subject;
        MatchArm::List arms;
    };

    struct PathExpr : ExprKind {
        PathExpr(Path && path)
            : ExprKind {ExprKind::Kind::Path}, path {std::move(path)} {}

        Path path;
    };

    struct PostfixExpr : ExprKind {
        PostfixExpr(Expr && lhs, PostfixOp op)
            : ExprKind {ExprKind::Kind::Loop}, lhs {std::move(lhs)}, op {op} {}

        Expr lhs;
        PostfixOp op;
    };

    struct PrefixExpr : ExprKind {
        PrefixExpr(PrefixOp op, Expr && rhs)
            : ExprKind {ExprKind::Kind::Loop}, op {op}, rhs {std::move(rhs)} {}

        PrefixOp op;
        Expr rhs;
    };

    struct ReturnExpr : ExprKind {
        ReturnExpr(Expr::Opt && value)
            : ExprKind {ExprKind::Kind::Return}, value {std::move(value)} {}

        Expr::Opt value;
    };

    struct TupleExpr : ExprKind {
        TupleExpr(ExprKind::List && values)
            : ExprKind {ExprKind::Kind::Tuple}, values {std::move(values)} {}

        ExprKind::List values;
    };
}

#endif // JACY_HIR_NODES_EXPRS_H
