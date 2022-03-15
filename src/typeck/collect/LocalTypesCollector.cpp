#include "LocalTypesCollector.h"

namespace jc::typeck {
    void LocalTypesCollector::visitLiteralExpr(const hir::LitExpr & literal, const hir::Expr::ExprData & data) {
        // TODO!: Suffixes
        tyCtx.addExprType(data.nodeId, getLitExprType(literal.kind));
    }

    Ty LocalTypesCollector::getLitExprType(const hir::LitExpr::Kind & kind) {
        switch (kind) {
            case ast::LitExpr::Kind::Bool: {
                return tyCtx.makeDefaultPrimTypeByKind(TypeKind::Kind::Bool);
            }
            case ast::LitExpr::Kind::Int: {
                return tyCtx.makeDefaultPrimTypeByKind(TypeKind::Kind::Int);
            }
            case ast::LitExpr::Kind::Float: {
                return tyCtx.makeDefaultPrimTypeByKind(TypeKind::Kind::Float);
            }
            case ast::LitExpr::Kind::Str: {
                return tyCtx.makeDefaultPrimTypeByKind(TypeKind::Kind::Str);
            }
        }
    }

    void LocalTypesCollector::visitBlockExpr(const hir::BlockExpr & block, const hir::Expr::ExprData & data) {
        const auto & lastStmt = block.block.stmts.back();
        if (lastStmt.kind->kind == hir::StmtKind::Kind::Expr) {
            const auto & lastExpr = hir::StmtKind::as<hir::ExprStmt>(lastStmt.kind);
            // Set last expression statement type as block type
            tyCtx.addExprType(data.nodeId, tyCtx.getExprType(lastExpr->expr.nodeId));
        } else {
            tyCtx.addExprType(data.nodeId, tyCtx.makeUnit());
        }
    }
}
