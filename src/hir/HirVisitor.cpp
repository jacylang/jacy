#include "hir/HirVisitor.h"

namespace jc::hir {
    void HirVisitor::visit(const Party & party) const {
        visitMod(party.rootMod);
    }

    void HirVisitor::visitMod(const Mod & mod) const {
        visitEach(mod.items);
    }

    void HirVisitor::visitItem(const ItemId & itemId) const {
        const auto & itemWrapper = party.item(itemId);
        const auto & item = itemWrapper.kind;
        visitItemKind(item);
    }

    void HirVisitor::visitItemKind(const ItemKind::Ptr & item) const {
        switch (item->kind) {
            case ItemKind::Kind::Const: {
                visitConst(*ItemKind::as<Const>(item));
                break;
            }
            case ItemKind::Kind::Enum: {
                visitEnum(*ItemKind::as<Enum>(item));
                break;
            }
            case ItemKind::Kind::Func: {
                visitFunc(*ItemKind::as<Func>(item));
                break;
            }
            case ItemKind::Kind::Impl: {
                visitImpl(*ItemKind::as<Impl>(item));
                break;
            }
            case ItemKind::Kind::Mod: {
                visitMod(*ItemKind::as<Mod>(item));
                break;
            }
            case ItemKind::Kind::Struct: {
                visitStruct(*ItemKind::as<Struct>(item));
                break;
            }
            case ItemKind::Kind::Trait: {
                visitTrait(*ItemKind::as<Trait>(item));
                break;
            }
            case ItemKind::Kind::TypeAlias: {
                visitTypeAlias(*ItemKind::as<TypeAlias>(item));
                break;
            }
            case ItemKind::Kind::Use: {
                visitUseDecl(*ItemKind::as<UseDecl>(item));
                break;
            }
        }
    }

    void HirVisitor::visitConst(const Const & constItem) const {
        visitType(constItem.type);
        visitBody(constItem.body);
    }

    void HirVisitor::visitVariant(const Variant & variant) const {}

    void HirVisitor::visitEnum(const Enum & enumItem) const {}

    void HirVisitor::visitFuncSig(const FuncSig & funcSig) const {}

    void HirVisitor::visitFunc(const Func & func) const {}

    void HirVisitor::visitImplMember(const ImplMember & implMember) const {}

    void HirVisitor::visitImpl(const Impl & impl) const {}

    void HirVisitor::visitStruct(const Struct & structItem) const {}

    void HirVisitor::visitTraitMember(const TraitMember & traitMember) const {}

    void HirVisitor::visitTrait(const Trait & trait) const {}

    void HirVisitor::visitTypeAlias(const TypeAlias & typeAlias) const {}

    void HirVisitor::visitUseDecl(const UseDecl & useDecl) const {}

    void HirVisitor::visitStmt(const Stmt & stmt) const {
        switch (stmt.kind->kind) {
            case StmtKind::Kind::Let:
                break;
            case StmtKind::Kind::Item:
                break;
            case StmtKind::Kind::Expr:
                break;
        }
    }

    void HirVisitor::visitExpr(const Expr & expr) const {
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
