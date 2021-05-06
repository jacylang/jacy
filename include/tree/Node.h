#ifndef JACY_NODE_H
#define JACY_NODE_H

#include "parser/Token.h"

#include <memory>
#include <utility>

namespace jc::tree {
    struct Node;
    using parser::Location;
    using node_ptr = std::shared_ptr<Node>;

    struct Node {
        explicit Node(const Location & loc) : loc(loc) {}

        parser::Location loc;
    };
}

#endif // JACY_NODE_H
