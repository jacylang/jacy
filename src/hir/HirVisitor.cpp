#include "hir/HirVisitor.h"

namespace jc::hir {
    void HirVisitor::visit(const Party & party) const {
        visit(party.rootMod);
    }

    void HirVisitor::visit(const Mod & mod) const {
        visitEach(mod.items);
    }

    void HirVisitor::visit(const ItemId & itemId) const {
        const auto & itemWrapper = party.item(itemId);

        const auto & item = itemWrapper.kind;

        switch (item->kind) {
            case ItemKind::Kind::Const:
                break;
            case ItemKind::Kind::Enum:
                break;
            case ItemKind::Kind::Func:
                break;
            case ItemKind::Kind::Impl:
                break;
            case ItemKind::Kind::Mod:
                break;
            case ItemKind::Kind::Struct:
                break;
            case ItemKind::Kind::Trait:
                break;
            case ItemKind::Kind::TypeAlias:
                break;
            case ItemKind::Kind::Use:
                break;
        }
    }

    void HirVisitor::visit(const Stmt & stmt) const {
        switch (stmt.kind->kind) {
            case StmtKind::Kind::Let:
                break;
            case StmtKind::Kind::Item:
                break;
            case StmtKind::Kind::Expr:
                break;
        }
    }

    void HirVisitor::visit(const Expr & expr) const {
        switch (expr.kind->kind) {
            case ExprKind::Kind::Array:
                break;
            case ExprKind::Kind::Assign:
                break;
            case ExprKind::Kind::Block:
                break;
            case ExprKind::Kind::Borrow:
                break;
            case ExprKind::Kind::Break:
                break;
            case ExprKind::Kind::Continue:
                break;
            case ExprKind::Kind::Deref:
                break;
            case ExprKind::Kind::Field:
                break;
            case ExprKind::Kind::If:
                break;
            case ExprKind::Kind::Infix:
                break;
            case ExprKind::Kind::Invoke:
                break;
            case ExprKind::Kind::Lambda:
                break;
            case ExprKind::Kind::List:
                break;
            case ExprKind::Kind::Literal:
                break;
            case ExprKind::Kind::Loop:
                break;
            case ExprKind::Kind::Match:
                break;
            case ExprKind::Kind::Path:
                break;
            case ExprKind::Kind::Prefix:
                break;
            case ExprKind::Kind::Return:
                break;
            case ExprKind::Kind::Self:
                break;
            case ExprKind::Kind::Subscript:
                break;
            case ExprKind::Kind::Tuple:
                break;
            case ExprKind::Kind::Unit:
                break;
        }
    }
}
