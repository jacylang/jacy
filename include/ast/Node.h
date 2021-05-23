#ifndef JACY_NODE_H
#define JACY_NODE_H

#include <memory>
#include <utility>
#include <cstdint>

#include "parser/Token.h"

namespace jc::ast {
    struct Node;
    using span::Span;
    using node_ptr = std::shared_ptr<Node>;
    using node_id = uint32_t;
    using opt_node_id = dt::Option<ast::node_id>;

    const node_id NONE_NODE_ID = UINT32_MAX;

    // TODO: Move to `Party` when multi-file will be done
    class NodeMap {
    public:
        NodeMap() = default;

        node_id addNode(Node * node) {
            if (currentNodeId == NONE_NODE_ID) {
                common::Logger::devPanic("Nodes count exceeded");
            }
            nodes.emplace(currentNodeId, node);
            return currentNodeId++;
        }

        const Node & getNode(node_id nodeId) const;

        const Span & getNodeSpan(node_id nodeId) const;

    private:
        node_id currentNodeId{0};
        std::map<node_id, Node*> nodes;
    };

    struct Node {
        explicit Node(const Span & span) : span(span) {
            id = nodeMap.addNode(this);
        }

        node_id id{NONE_NODE_ID};
        Span span;

        static NodeMap nodeMap;
    };
}

#endif // JACY_NODE_H
