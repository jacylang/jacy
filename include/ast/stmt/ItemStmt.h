#ifndef JACY_AST_STMT_ITEMSTMT_H
#define JACY_AST_STMT_ITEMSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/item/Item.h"

namespace jc::ast {
    struct ItemStmt : Stmt {
        explicit ItemStmt(Item::Ptr && item, const Span & span)
            : Stmt{span, StmtKind::Item}, item{std::move(item)} {}

        Item::Ptr item;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_ITEMSTMT_H
