#include "hir/HirVisitor.h"

namespace jc::hir {
    void HirVisitor::visit(const Party & party) const {
        visitMod(party.rootMod);
    }

    // Item //
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

    void HirVisitor::visitMod(const Mod & mod) const {
        visitEach(mod.items);
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

    // Stmt //
    void HirVisitor::visitStmt(const Stmt & stmt) const {
        const auto & stmtKind = stmt.kind;
        visitStmtKind(stmtKind);
    }

    void HirVisitor::visitStmtKind(const StmtKind::Ptr & stmt) const {
        switch (stmt->kind) {
            case StmtKind::Kind::Let: {
                visitLetStmt(*StmtKind::as<LetStmt>(stmt));
                break;
            }
            case StmtKind::Kind::Item: {
                visitItemStmt(*StmtKind::as<ItemStmt>(stmt));
                break;
            }
            case StmtKind::Kind::Expr: {
                visitExprStmt(*StmtKind::as<ExprStmt>(stmt));
                break;
            }
        }
    }

    void HirVisitor::visitLetStmt(const LetStmt & letStmt) const {}

    void HirVisitor::visitItemStmt(const ItemStmt & itemStmt) const {}

    void HirVisitor::visitExprStmt(const ExprStmt & exprStmt) const {}

    // Expr //
    void HirVisitor::visitExpr(const Expr & expr) const {
        const auto & exprKind = expr.kind;
        visitExprKind(exprKind);
    }

    void HirVisitor::visitExprKind(const ExprKind::Ptr & expr) const {
        switch (expr->kind) {
            case ExprKind::Kind::Array: {
                visitArrayExpr(*ExprKind::as<ArrayExpr>(expr));
                break;
            }
            case ExprKind::Kind::Assign: {
                visitAssignExpr(*ExprKind::as<AssignExpr>(expr));
                break;
            }
            case ExprKind::Kind::Block: {
                visitBlockExpr(*ExprKind::as<BlockExpr>(expr));
                break;
            }
            case ExprKind::Kind::Borrow: {
                visitBorrowExpr(*ExprKind::as<BorrowExpr>(expr));
                break;
            }
            case ExprKind::Kind::Break: {
                visitBreakExpr(*ExprKind::as<BreakExpr>(expr));
                break;
            }
            case ExprKind::Kind::Continue: {
                visitContinueExpr(*ExprKind::as<ContinueExpr>(expr));
                break;
            }
            case ExprKind::Kind::Deref: {
                visitDerefExpr(*ExprKind::as<DerefExpr>(expr));
                break;
            }
            case ExprKind::Kind::Field: {
                visitFieldExpr(*ExprKind::as<FieldExpr>(expr));
                break;
            }
            case ExprKind::Kind::If: {
                visitIfExpr(*ExprKind::as<IfExpr>(expr));
                break;
            }
            case ExprKind::Kind::Infix: {
                visitInfixExpr(*ExprKind::as<InfixExpr>(expr));
                break;
            }
            case ExprKind::Kind::Invoke: {
                visitInvokeExpr(*ExprKind::as<InvokeExpr>(expr));
                break;
            }
            case ExprKind::Kind::Lambda: {
                visitLambdaExpr(*ExprKind::as<LambdaExpr>(expr));
                break;
            }
            case ExprKind::Kind::List: {
                visitListExpr(*ExprKind::as<ListExpr>(expr));
                break;
            }
            case ExprKind::Kind::Literal: {
                visitLiteralExpr(*ExprKind::as<LitExpr>(expr));
                break;
            }
            case ExprKind::Kind::Loop: {
                visitLoopExpr(*ExprKind::as<LoopExpr>(expr));
                break;
            }
            case ExprKind::Kind::Match: {
                visitMatchExpr(*ExprKind::as<MatchExpr>(expr));
                break;
            }
            case ExprKind::Kind::Path: {
                visitPathExpr(*ExprKind::as<PathExpr>(expr));
                break;
            }
            case ExprKind::Kind::Prefix: {
                visitPrefixExpr(*ExprKind::as<PrefixExpr>(expr));
                break;
            }
            case ExprKind::Kind::Return: {
                visitReturnExpr(*ExprKind::as<ReturnExpr>(expr));
                break;
            }
            case ExprKind::Kind::Self: {
                visitSelfExpr(*ExprKind::as<SelfExpr>(expr));
                break;
            }
            case ExprKind::Kind::Subscript: {
                visitSubscriptExpr(*ExprKind::as<Subscript>(expr));
                break;
            }
            case ExprKind::Kind::Tuple: {
                visitTupleExpr(*ExprKind::as<TupleExpr>(expr));
                break;
            }
            case ExprKind::Kind::Unit: {
                visitUnitExpr(*ExprKind::as<UnitExpr>(expr));
                break;
            }
        }
    }

    void HirVisitor::visitArrayExpr(const ArrayExpr & array) const {
    }

    void HirVisitor::visitAssignExpr(const AssignExpr & assign) const {
    }

    void HirVisitor::visitBlockExpr(const BlockExpr & block) const {
    }

    void HirVisitor::visitBorrowExpr(const BorrowExpr & borrow) const {
    }

    void HirVisitor::visitBreakExpr(const BreakExpr & breakExpr) const {
    }

    void HirVisitor::visitContinueExpr(const ContinueExpr & continueExpr) const {
    }

    void HirVisitor::visitDerefExpr(const DerefExpr & deref) const {
    }

    void HirVisitor::visitFieldExpr(const FieldExpr & field) const {
    }

    void HirVisitor::visitIfExpr(const IfExpr & ifExpr) const {
    }

    void HirVisitor::visitInfixExpr(const InfixExpr & infix) const {
    }

    void HirVisitor::visitInvokeExpr(const InvokeExpr & invoke) const {
    }

    void HirVisitor::visitLambdaExpr(const LambdaExpr & lambda) const {
    }

    void HirVisitor::visitListExpr(const ListExpr & list) const {
    }

    void HirVisitor::visitLiteralExpr(const LitExpr & literal) const {
    }

    void HirVisitor::visitLoopExpr(const LoopExpr & loop) const {
    }

    void HirVisitor::visitMatchExpr(const MatchExpr & match) const {
    }

    void HirVisitor::visitPathExpr(const PathExpr & path) const {
    }

    void HirVisitor::visitPrefixExpr(const PrefixExpr & prefix) const {
    }

    void HirVisitor::visitReturnExpr(const ReturnExpr & returnExpr) const {
    }

    void HirVisitor::visitSelfExpr(const SelfExpr & self) const {
    }

    void HirVisitor::visitSubscriptExpr(const Subscript & subscript) const {
    }

    void HirVisitor::visitTupleExpr(const TupleExpr & tuple) const {
    }

    void HirVisitor::visitUnitExpr(const UnitExpr & unit) const {
    }

    // Type //
    void HirVisitor::visitType(const Type & type) const {
        const auto & typeKind = type.kind;
        visitTypeKind(typeKind);
    }

    void HirVisitor::visitTypeKind(const TypeKind::Ptr & type) const {
        switch (type->kind) {
            case TypeKind::Kind::Infer: {
                // ?
                break;
            }
            case TypeKind::Kind::Tuple: {
                visitTupleType(*TypeKind::as<TupleType>(type));
                break;
            }
            case TypeKind::Kind::Func: {
                visitFuncType(*TypeKind::as<FuncType>(type));
                break;
            }
            case TypeKind::Kind::Slice: {
                visitSliceType(*TypeKind::as<SliceType>(type));
                break;
            }
            case TypeKind::Kind::Array: {
                visitArrayType(*TypeKind::as<ArrayType>(type));
                break;
            }
            case TypeKind::Kind::Path: {
                visitPathType(*TypeKind::as<TypePath>(type));
                break;
            }
            case TypeKind::Kind::Unit: {
                visitUnitType(*TypeKind::as<UnitType>(type));
                break;
            }
        }
    }

    void HirVisitor::visitTupleType(const TupleType & tupleType) const {}

    void HirVisitor::visitFuncType(const FuncType & funcType) const {}

    void HirVisitor::visitSliceType(const SliceType & sliceType) const {}

    void HirVisitor::visitArrayType(const ArrayType & arrayType) const {}

    void HirVisitor::visitPathType(const TypePath & typePath) const {}

    void HirVisitor::visitUnitType(const UnitType & unitType) const {}
}
