#ifndef JACY_AST_NODEMAP_H
#define JACY_AST_NODEMAP_H

#include "ast/Node.h"

namespace jc::ast {
    class NodeMap {
    public:
        NodeMap() = default;

        template<class T, class ...Args>
        N<T> makeBoxNode(Args && ...args) {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            addNode(node.get());
            return node;
        }

        template<class T>
        T * addNode(T * node) {
            checkSize();
            node->id = static_cast<node_id>(nodes.size());
            nodes.emplace_back(node);
            return node;
        }

        const Node & getNode(node_id nodeId) const;
        const Span & getNodeSpan(node_id nodeId) const;
        size_t size() const {
            return nodes.size();
        }

        void checkSize() const {
            if (nodes.size() == NONE_NODE_ID) {
                common::Logger::devPanic("Nodes count exceeded");
            }
        }

    private:
        std::vector<const Node*> nodes;
    };
}

#endif // JACY_AST_NODEMAP_H
