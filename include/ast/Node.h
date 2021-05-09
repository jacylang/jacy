#ifndef JACY_NODE_H
#define JACY_NODE_H

#include <memory>
#include <utility>

#include "parser/Token.h"

namespace jc::ast {
    struct Node;
    using parser::Location;
    using node_ptr = std::shared_ptr<Node>;

    struct Node {
        explicit Node(const Location & loc) : loc(loc) {}

        parser::Location loc;
    };
}

#endif // JACY_NODE_H
