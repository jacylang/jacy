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

    struct ArrayExpr : Expr {
        ArrayExpr(Expr::List && elements)
            : Expr {Expr::Kind::Array}, elements {std::move(elements)} {}

        Expr::List elements;
    };

    struct AssignExpr : Expr {
        AssignExpr(ExprWrapper && lhs, const parser::Token & op, ExprWrapper && rhs)
            : Expr {Expr::Kind::Assign}, lhs {std::move(lhs)}, op {op}, rhs {std::move(rhs)} {}

        ExprWrapper lhs;
        parser::Token op;
        ExprWrapper rhs;
    };

    struct BlockExpr : Expr {
        BlockExpr(Block && block)
            : Expr {Expr::Kind::Block}, block {std::move(block)} {}

        Block block;
    };

    struct BorrowExpr : Expr {
        BorrowExpr(bool mut, ExprWrapper && rhs)
            : Expr {Expr::Kind::Borrow}, mut {mut}, rhs {std::move(rhs)} {}

        bool mut;
        ExprWrapper rhs;
    };

    struct BreakExpr : Expr {
        BreakExpr(ExprWrapper::Opt && value) : Expr {Expr::Kind::Break}, value {std::move(value)} {}

        ExprWrapper::Opt value;
    };

    struct ContinueExpr : Expr {
        ContinueExpr() : Expr {Expr::Kind::Continue} {}
    };

    struct DerefExpr : Expr {
        DerefExpr(ExprWrapper::Opt && rhs) : Expr {Expr::Kind::Deref}, rhs {std::move(rhs)} {}

        ExprWrapper::Opt rhs;
    };

    struct FieldExpr : Expr {
        FieldExpr(
            ExprWrapper && lhs,
            const span::Ident & field
        ) : Expr {Expr::Kind::Field},
            lhs {std::move(lhs)},
            field {field} {}

        ExprWrapper lhs;
        span::Ident field;
    };

    struct IfExpr : Expr {
        IfExpr(
            ExprWrapper && cond,
            Block::Opt && ifBranch,
            Block::Opt && elseBranch
        ) : Expr {Expr::Kind::If},
            cond {std::move(cond)},
            ifBranch {std::move(ifBranch)},
            elseBranch {std::move(elseBranch)} {}

        ExprWrapper cond;
        Block::Opt ifBranch;
        Block::Opt elseBranch;
    };

    struct InfixExpr : Expr {
        InfixExpr(ExprWrapper && lhs, BinOp op, ExprWrapper && rhs)
            : Expr {Expr::Kind::Invoke}, lhs {std::move(lhs)}, op {op}, rhs {std::move(rhs)} {}

        ExprWrapper lhs;
        BinOp op;
        ExprWrapper rhs;
    };

    struct InvokeExpr : Expr {
        InvokeExpr(ExprWrapper && lhs, Arg::List && args)
            : Expr {Expr::Kind::Invoke}, lhs {std::move(lhs)}, args {std::move(args)} {}

        ExprWrapper lhs;
        Arg::List args;
    };

    struct LitExpr : Expr {
        using Kind = ast::LitExpr::Kind;
        using ValueT = ast::LitExpr::ValueT;

        LitExpr(Kind kind, ValueT val, parser::Token token)
            : Expr {Expr::Kind::Literal}, kind {kind}, val {val}, token {token} {}

        Kind kind;
        ValueT val;

        // TODO: Maybe remove token and add to-token-conversion API
        parser::Token token;
    };

    struct LoopExpr : Expr {
        LoopExpr(Block && body)
            : Expr {Expr::Kind::Loop}, body {std::move(body)} {}

        Block body;
    };

    struct MatchArm : HirNode {
        using List = std::vector<MatchArm>;

        MatchArm(Pat::Ptr && pat, ExprWrapper && body, HirId hirId, Span span)
            : hirId {hirId},
              span {span},
              pat {std::move(pat)},
              body {std::move(body)} {}

        HirId hirId;
        Span span;
        Pat::Ptr pat;
        ExprWrapper body;
    };

    struct MatchExpr : Expr {
        MatchExpr(ExprWrapper && expr, MatchArm::List && arms)
            : Expr {Expr::Kind::Match},
              subject {std::move(expr)},
              arms {std::move(arms)} {}

        ExprWrapper subject;
        MatchArm::List arms;
    };

    struct PathExpr : Expr {
        PathExpr(Path && path)
            : Expr {Expr::Kind::Path}, path {std::move(path)} {}

        Path path;
    };

    struct PostfixExpr : Expr {
        PostfixExpr(ExprWrapper && lhs, PostfixOp op)
            : Expr {Expr::Kind::Loop}, lhs {std::move(lhs)}, op {op} {}

        ExprWrapper lhs;
        PostfixOp op;
    };

    struct PrefixExpr : Expr {
        PrefixExpr(PrefixOp op, ExprWrapper && rhs)
            : Expr {Expr::Kind::Loop}, op {op}, rhs {std::move(rhs)} {}

        PrefixOp op;
        ExprWrapper rhs;
    };

    struct ReturnExpr : Expr {
        ReturnExpr(ExprWrapper::Opt && value)
            : Expr {Expr::Kind::Return}, value {std::move(value)} {}

        ExprWrapper::Opt value;
    };

    struct TupleExpr : Expr {
        TupleExpr(Expr::List && values)
            : Expr {Expr::Kind::Tuple}, values {std::move(values)} {}

        Expr::List values;
    };
}

#endif // JACY_HIR_NODES_EXPRS_H
