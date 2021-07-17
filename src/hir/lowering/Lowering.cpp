#include "hir/lowering/Lowering.h"

namespace jc::hir {
    void Lowering::lower(const sess::sess_ptr & sess, const ast::Party & party) {
        this->sess = sess;


    }

    // Expressions //
    expr_ptr Lowering::lowerExpr(const ast::expr_ptr & exprPr) {
        auto expr = exprPr.unwrap().get();
        switch (expr->kind) {
            case ast::ExprKind::Assign:
                return lowerAssignExpr(expr->as<ast::Assignment>(expr));
            case ast::ExprKind::Block:
                break;
            case ast::ExprKind::Borrow:
                break;
            case ast::ExprKind::Break:
                break;
            case ast::ExprKind::Continue:
                break;
            case ast::ExprKind::Deref:
                break;
            case ast::ExprKind::Id:
                break;
            case ast::ExprKind::If:
                break;
            case ast::ExprKind::Infix:
                break;
            case ast::ExprKind::Invoke:
                break;
            case ast::ExprKind::Lambda:
                break;
            case ast::ExprKind::List:
                break;
            case ast::ExprKind::LiteralConstant:
                break;
            case ast::ExprKind::Loop:
                break;
            case ast::ExprKind::MemberAccess:
                break;
            case ast::ExprKind::Paren:
                break;
            case ast::ExprKind::Path:
                break;
            case ast::ExprKind::Prefix:
                break;
            case ast::ExprKind::Quest:
                break;
            case ast::ExprKind::Return:
                break;
            case ast::ExprKind::Spread:
                break;
            case ast::ExprKind::Struct:
                break;
            case ast::ExprKind::Subscript:
                break;
            case ast::ExprKind::Super:
                break;
            case ast::ExprKind::This:
                break;
            case ast::ExprKind::Tuple:
                break;
            case ast::ExprKind::Unit:
                break;
            case ast::ExprKind::When:
                break;
        }
    }

    expr_ptr Lowering::lowerAssignExpr(const ast::Assignment & assign) {
        return makeBoxNode<Assign>(
            lowerExpr(assign.lhs),
            assign.op,
            lowerExpr(assign.rhs),
            NONE_HIR_ID,
            assign.span
        );
    }

    expr_ptr Lowering::lowerBlockExpr(const ast::Block & astBlock) {
        auto block = lowerBlock(astBlock);
        return makeBoxNode<BlockExpr>(block, block.hirId, block.span);
    }

    // Statements //
    stmt_ptr Lowering::lowerStmt(const ast::stmt_ptr & astStmt) {
        const auto & stmt = astStmt.unwrap();
        switch (stmt->kind) {
            case ast::StmtKind::Expr: return lowerExprStmt(stmt->as<ast::ExprStmt>(stmt.get()));
            case ast::StmtKind::For:
                break;
            case ast::StmtKind::Var:
                break;
            case ast::StmtKind::While:
                break;
            case ast::StmtKind::Item:
                break;
        }
    }

    stmt_ptr Lowering::lowerExprStmt(const ast::ExprStmt & exprStmt) {
        return makeBoxNode<ExprStmt>(lowerExpr(exprStmt.expr), NONE_HIR_ID, exprStmt.span);
    }

    // Fragments //
    Block Lowering::lowerBlock(const ast::Block & block) {
        // FIXME: One-line blocks will be removed!
        stmt_list stmts;
        for (const auto & stmt : block.stmts.unwrap()) {
            stmts.emplace_back(lowerStmt(stmt));
        }
        return Block(std::move(stmts), NONE_HIR_ID, block.span);
    }
}
