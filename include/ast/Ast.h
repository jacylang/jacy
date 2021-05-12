#ifndef JACY_AST_AST_H
#define JACY_AST_AST_H

#include "ast/nodes.h"
#include "common/Logger.h"

namespace jc::ast {
    class Ast {
    public:
        Ast(bool wellFormed, stmt_list && tree) : wellFormed(wellFormed), tree(std::move(tree)) {}

        const stmt_list & getTree() const {
            if (!wellFormed) {
                common::Logger::devPanic("Tried to get ill-formed AST tree from `Ast`");
            }
            return tree;
        }

    private:
        bool wellFormed;
        stmt_list tree;
    };
}

#endif // JACY_AST_AST_H
