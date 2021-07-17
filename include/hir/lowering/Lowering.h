#ifndef JACY_HIR_LOWERING_H
#define JACY_HIR_LOWERING_H

#include "ast/nodes.h"
#include "session/Session.h"
#include "hir/nodes/nodes.h"

namespace jc::hir {
    class Lowering {
    public:
        Lowering() = default;
        virtual ~Lowering() = default;

        void lower(const sess::sess_ptr & sess, const ast::Party & party);

    private:
        template<class T, class ...Args>
        N<T> makeBoxNode(Args ...args) {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }


    private:
        expr_ptr lowerExpr(const ast::expr_ptr & expr);
        expr_ptr lowerAssignExpr(const ast::Assignment & assign);
        expr_ptr lowerBlockExpr(const ast::Block & block);

    private:
        stmt_ptr lowerStmt(const ast::stmt_ptr & stmt);

    private:
        Block lowerBlock(const ast::Block & block);

    private:
        sess::sess_ptr sess;
    };
}

#endif // JACY_HIR_LOWERING_H
