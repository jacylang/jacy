#ifndef JACY_AST_NODEMAP_H
#define JACY_AST_NODEMAP_H

#include "ast/Node.h"

namespace jc::ast {
    class NodeMap {
    public:
        NodeMap() = default;

        template<class T>
        const T * addNode(const Node * node) {
            if (currentNodeId == NONE_NODE_ID) {
                common::Logger::devPanic("Nodes count exceeded");
            }
            node->id = currentNodeId;
            nodes.emplace(currentNodeId, node);
            currentNodeId++;
            return node;
        }

        const Node & getNode(node_id nodeId) const;
        const Span & getNodeSpan(node_id nodeId) const;
        node_ptr getNodePtr(node_id nodeId) const;
        size_t size() const {
            return nodes.size();
        }

    private:
        // FIXME: Just use vector
        node_id currentNodeId{0};
        std::map<node_id, const Node*> nodes;
    };
}

#endif //JACY_AST_NODEMAP_H
