#ifndef JACY_AST_STMT_ITEM_H
#define JACY_AST_STMT_ITEM_H

#include "ast/stmt/Stmt.h"
#include "ast/fragments/Attribute.h"

namespace jc::ast {
    struct Item;
    using item_ptr = std::shared_ptr<Item>;
    using item_list = std::vector<item_ptr>;

    struct Item : Stmt {
        Item(attr_list attributes, stmt_ptr stmt, const Span & span)
            : attributes(std::move(attributes)), stmt(std::move(stmt)), Stmt(span, StmtKind::Item) {}

        attr_list attributes;
        stmt_ptr stmt;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_STMT_ITEM_H
