#ifndef JACY_NODE_H
#define JACY_NODE_H

#include <memory>
#include <utility>

#include "parser/Token.h"

namespace jc::ast {
    struct Node;
    using span::Span;
    using node_ptr = std::shared_ptr<Node>;

    struct Node {
        explicit Node(const Span & span) : span(span) {}

        Span span;
    };
}

#endif // JACY_NODE_H
