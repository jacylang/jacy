#ifndef JACY_SRC_TYPECK_COLLECT_LOCALTYPESCOLLECTOR_H
#define JACY_SRC_TYPECK_COLLECT_LOCALTYPESCOLLECTOR_H

#include "hir/nodes/Party.h"
#include "hir/HirVisitor.h"
#include "session/Session.h"

namespace jc::typeck {
    /**
     * @brief Collects types of expressions and let statements (if have an annotation)
     */
    class LocalTypesCollector : public hir::HirVisitor {
    public:
        LocalTypesCollector(const hir::Party & party, const sess::Session::Ptr & sess)
            : hir::HirVisitor {party},
              sess {sess},
              tyCtx {sess->tyCtx} {}

        virtual ~LocalTypesCollector() = default;

        // Expressions //
    public:
        void visitLiteralExpr(const hir::LitExpr & literal, const hir::Expr::ExprData & data) override;

        Ty getLitExprType(hir::LitExpr::Kind kind);

        void visitBlockExpr(const hir::BlockExpr & block, const hir::Expr::ExprData & data) override;

        // Locals //
    public:
        void visitLetStmt(const hir::LetStmt & letStmt) override;

    private:
        sess::Session::Ptr sess;
        TypeContext & tyCtx;
    };
}

#endif //JACY_SRC_TYPECK_COLLECT_LOCALTYPESCOLLECTOR_H
