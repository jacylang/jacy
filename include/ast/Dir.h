#ifndef JACY_INCLUDE_AST_DIR_H
#define JACY_INCLUDE_AST_DIR_H

#include "ast/Node.h"

namespace jc::ast {
    struct Dir : Node {
        Dir(
            const std::string & name,
            std::vector<node_ptr> && modules
        ) : Node(Span{}),
            name(name),
            modules(std::move(modules)) {}

        std::string name;
        std::vector<node_ptr> modules;
    };
}

#endif // JACY_INCLUDE_AST_DIR_H
