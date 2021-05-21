#ifndef JACY_NODE_H
#define JACY_NODE_H

#include <memory>
#include <utility>

#include "parser/Token.h"

namespace jc::ast {
    struct Node;
    using span::Span;
    using node_ptr = std::shared_ptr<Node>;
    using node_id = uint32_t;

    class NodeMap {
    public:
        NodeMap() = default;

        void addNode(Node * node) {
            lastNodeId++;
            nodes.emplace(lastNodeId, node);
        }

        const Node & getNode(node_id nodeId) const {
            return *nodes.at(nodeId);
        }

    private:
        node_id lastNodeId{0};
        std::map<node_id, Node*> nodes;
    };

    struct Node {
        explicit Node(const Span & span) : span(span) {
            nodeMap.addNode(this);
        }

        Span span;

        static NodeMap nodeMap;
    };
}

#endif // JACY_NODE_H
