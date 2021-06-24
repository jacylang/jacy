#ifndef JACY_INCLUDE_AST_DIR_H
#define JACY_INCLUDE_AST_DIR_H

#include "ast/Node.h"

namespace jc::ast {
    struct Dir;
    using dir_ptr = std::shared_ptr<Dir>;

    struct Dir : Node {
        Dir(
            const std::string & name,
            node_list && modules
        ) : Node(Span{}),
            name(name),
            modules(std::move(modules)) {}

        std::string name;
        node_list modules;
    };
}

#endif // JACY_INCLUDE_AST_DIR_H
