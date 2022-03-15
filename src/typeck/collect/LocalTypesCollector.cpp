#include "LocalTypesCollector.h"

namespace jc::typeck {
    void LocalTypesCollector::visitLiteralExpr(const hir::LitExpr & literal) {
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
}
