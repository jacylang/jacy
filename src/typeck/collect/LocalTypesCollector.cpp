#include "LocalTypesCollector.h"

namespace jc::typeck {
    void LocalTypesCollector::visitLiteralExpr(const hir::LitExpr & literal, const hir::Expr::ExprData & data) {
        // TODO!: Suffixes
        tyCtx.addExprType(data.nodeId, getLitExprType(literal.kind));
    }

    Ty LocalTypesCollector::getLitExprType(hir::LitExpr::Kind kind) {
        // TODO: Remove `makeDefaultPrimTypeByKind` and use `Infer`
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
        HirVisitor::visitBlockExpr(block, data);

        const auto & lastStmt = block.block.stmts.back();
        if (lastStmt.kind->kind == hir::StmtKind::Kind::Expr) {
            const auto & lastExpr = hir::StmtKind::as<hir::ExprStmt>(lastStmt.kind);
            // Set last expression statement type as block type
            tyCtx.addExprType(data.nodeId, tyCtx.getExprType(lastExpr->expr.nodeId));
        } else {
            tyCtx.addExprType(data.nodeId, tyCtx.makeUnit());
        }
    }

    // Locals //
    void LocalTypesCollector::visitLetStmt(const hir::LetStmt & letStmt) {
        // Visit let statement inners before setting its type. This will collect value type if some is present.
        HirVisitor::visitLetStmt(letStmt);

        auto localNodeId = letStmt.pat.nodeId;

        Ty type;
        if (letStmt.type.some()) {
            type = tyCtx.converter().convert(letStmt.type.unwrap());
        } else if (letStmt.value.some()) {
            // TODO: Is this okay?
            //  Set type of local if initializer is present if expression might be of unknown type?
            //  I think it is possible because even if expression is not inferred it is a type variable,
            //  thus will be replaced with a specific type if no error appear.
            type = tyCtx.getExprType(letStmt.value.unwrap().nodeId);
        } else {
            type = tyCtx.makeInferVar();
        }

        tyCtx.addLocalType(localNodeId, type);
    }
}
