#include "ast/Node.h"

namespace jc::ast {
    NodeMap Node::nodeMap{};

    const Node & NodeMap::getNode(node_id nodeId) const {
        return *nodes.at(nodeId);
    }

    const Span & NodeMap::getNodeSpan(node_id nodeId) const {
        return getNode(nodeId).span;
    }
}
