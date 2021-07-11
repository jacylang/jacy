#ifndef JACY_AST_NODEMAP_H
#define JACY_AST_NODEMAP_H

#include "ast/Node.h"

namespace jc::ast {
    class NodeMap {
    public:
        NodeMap() = default;

        template<class T, class ...Args>
        N<T> makeNode(Args && ...args) {
            if (nodes.size() == NONE_NODE_ID) {
                common::Logger::devPanic("Nodes count exceeded");
            }
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            node->id = nodes.size();
            nodes.emplace_back(node.get());
            return node;
        }

        const Node & getNode(node_id nodeId) const;
        const Span & getNodeSpan(node_id nodeId) const;
        size_t size() const {
            return nodes.size();
        }

    private:
        std::vector<const Node*> nodes;
    };
}

#endif // JACY_AST_NODEMAP_H
