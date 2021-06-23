#ifndef JACY_INCLUDE_AST_DIR_H
#define JACY_INCLUDE_AST_DIR_H

#include "ast/Node.h"

namespace jc::ast {
    struct Dir : Node {
        Dir(const std::string & name, const ) {}

        std::string name;
        std::vector<module_ptr> modules;
    };
}

#endif // JACY_INCLUDE_AST_DIR_H
