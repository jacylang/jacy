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

        dt::SuggResult<Party> lower(const sess::Session::Ptr & sess, const ast::Party & party);

    private:
        template<class T, class ...Args>
        N<T> makeBoxNode(Args ...args) {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }

        // Items //
    private:
        ItemNode lowerItem(const ast::Item::Ptr & astItem);
        Item::Ptr lowerEnum(const ast::Enum & astEnum);
        Variant lowerVariant(const ast::EnumEntry & enumEntry);
        Item::Ptr lowerMod(const ast::Item::List & astItems);
        Item::Ptr lowerFunc(const ast::Func & astFunc);

        // Statements //
    private:
        Stmt::Ptr lowerStmt(const ast::Stmt::Ptr & astStmt);
        Stmt::Ptr lowerExprStmt(const ast::ExprStmt & exprStmt);

        // Expressions //
    private:
        Expr::Ptr lowerExpr{const ast::Expr::Ptr & expr};
        Expr::Ptr lowerAssignExpr{const ast::Assign & assign};
        Expr::Ptr lowerBlockExpr{const ast::Block & block};

        // Types //
    private:
        Type::Ptr lowerType(const ast::Type::Ptr & astType);

        // Fragments //
    private:
        Block lowerBlock(const ast::Block & block);
        Body lowerBody(const ast::Body & astBody);

        // States //
    private:
        Party::ItemMap items;

    private:
        sess::Session::Ptr sess;
    };
}

#endif // JACY_HIR_LOWERING_H
