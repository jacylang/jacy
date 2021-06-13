#include "ast/NodeMap.h"

namespace jc::ast {
    const Node & NodeMap::getNode(node_id nodeId) const {
        // TODO: Return None if not found or panic?
        return *nodes.at(nodeId);
    }

    const Span & NodeMap::getNodeSpan(node_id nodeId) const {
        return getNode(nodeId).span;
    }

    node_ptr NodeMap::getNodePtr(node_id nodeId) const {
        return nodes.at(nodeId);
    }
}
