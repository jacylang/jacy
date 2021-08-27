#ifndef JACY_HIR_LOWERING_H
#define JACY_HIR_LOWERING_H

#include "ast/nodes.h"
#include "session/Session.h"
#include "hir/nodes/nodes.h"
#include "data_types/SuggResult.h"
#include "suggest/SuggInterface.h"

namespace jc::hir {
    class Lowering : public sugg::SuggInterface {
    public:
        Lowering() = default;
        virtual ~Lowering() = default;

        dt::SuggResult<Party> lower(const sess::sess_ptr & sess, const ast::Party & party);

    private:
        template<class T, class ...Args>
        N<T> makeBoxNode(Args ...args) {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }

        // Items //
    private:
        ItemNode lowerItem(const ast::item_ptr & astItem);
        item_ptr lowerEnum(const ast::Enum & astEnum);
        Variant lowerVariant(const ast::EnumEntry & enumEntry);
        item_ptr lowerMod(const ast::item_list & astItems);
        item_ptr lowerFunc(const ast::Func & astFunc);

        // Statements //
    private:
        stmt_ptr lowerStmt(const ast::StmtPtr & astStmt);
        stmt_ptr lowerExprStmt(const ast::ExprStmt & exprStmt);

        // Expressions //
    private:
        expr_ptr lowerExpr(const ast::Expr::Ptr & expr);
        expr_ptr lowerAssignExpr(const ast::Assign & assign);
        expr_ptr lowerBlockExpr(const ast::Block & block);

        // Types //
    private:
        Type::Ptr lowerType(const ast::Type::Ptr & astType);

        // Fragments //
    private:
        Block lowerBlock(const ast::Block & block);
        Body lowerBody(const ast::Body & astBody);

        // States //
    private:
        item_map items;

    private:
        sess::sess_ptr sess;
    };
}

#endif // JACY_HIR_LOWERING_H
