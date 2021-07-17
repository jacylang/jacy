#include "hir/lowering/Lowering.h"

namespace jc::hir {
    void Lowering::lower(const sess::sess_ptr & sess, const ast::Party & party) {

    }

    expr_ptr Lowering::lowerExpr(const ast::N<ast::Expr> & expr) {
        switch (expr->kind) {
            case ast::ExprKind::Assign: {

                break;
            }
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
}
