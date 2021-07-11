#ifndef JACY_AST_NODEMAP_H
#define JACY_AST_NODEMAP_H

#include "ast/Node.h"

namespace jc::ast {
    class NodeMap {
    public:
        NodeMap() = default;

        template<class T>
        node_id addNode(const T * node) {
            if (nodes.size() == NONE_NODE_ID) {
                common::Logger::devPanic("Nodes count exceeded");
            }
            nodes.emplace_back(node);
            return static_cast<node_id>(nodes.size() - 1);
        }

        const Node & getNode(node_id nodeId) const;
        const Span & getNodeSpan(node_id nodeId) const;
        node_ptr getNodePtr(node_id nodeId) const;
        size_t size() const {
            return nodes.size();
        }

    private:
        std::vector<const Node*> nodes;
    };
}

#endif //JACY_AST_NODEMAP_H
