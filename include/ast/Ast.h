#ifndef JACY_AST_AST_H
#define JACY_AST_AST_H

#include "ast/nodes.h"

namespace jc::ast {
    class Ast {
    public:
        Ast(stmt_list && tree) : tree(std::move(tree)) {}

        const stmt_list & getTree() const {
            return tree;
        }

    private:
        stmt_list tree;
    };
}

#endif // JACY_AST_AST_H
