#ifndef JACY_SRC_HIR_HIRVISITOR_H
#define JACY_SRC_HIR_HIRVISITOR_H

#include "hir/nodes/Party.h"

namespace jc::hir {
    class HirVisitor {
    public:
        HirVisitor(const Party & party) : party {party} {}

        virtual ~HirVisitor() = default;

        virtual void visit(const Party & party) const;

    private:
        virtual void visit(const Mod & mod) const;
        virtual void visit(const ItemId & itemId) const;

        virtual void visit(const Stmt & stmt) const;

        virtual void visit(const Expr & expr) const;

    private:
        template<class T>
        void visitEach(const std::vector<T> & list) const {
            for (const auto & el : list) {
                visit(el);
            }
        }

    private:
        const hir::Party & party;
    };
}

#endif // JACY_SRC_HIR_HIRVISITOR_H
