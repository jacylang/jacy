#ifndef JACY_AST_STMT_ITEMSTMT_H
#define JACY_AST_STMT_ITEMSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/item/Item.h"

namespace jc::ast {
    struct ItemStmt : Stmt {
        explicit ItemStmt(item_ptr item) : item(std::move(item)), Stmt(item->span, StmtKind::Item) {}

        item_ptr item;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_ITEMSTMT_H
