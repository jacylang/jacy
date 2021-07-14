#ifndef JACY_SESSION_H
#define JACY_SESSION_H

#include <string>
#include <memory>
#include <vector>
#include <random>

#include "common/Logger.h"
#include "session/SourceMap.h"
#include "resolve/DefStorage.h"

namespace jc::sess {
    struct Session;
    using sess_ptr = std::shared_ptr<Session>;

    class NodeStorage {
    public:
        template<class T>
        void addNode(T & node) {
            node.id = nextNodeId++;
            nodeSpanMap.emplace(node.id, node.span);
        }

        template<class T>
        void addNode(ast::N<T> & node) {
            node->id = nextNodeId++;
            nodeSpanMap.emplace(node->id, node->span);
        }

        const span::Span & getNodeSpan(ast::node_id nodeId) const {
            return nodeSpanMap.at(nodeId);
        }

    private:
        ast::node_id nextNodeId = 1; // Reserve `0` for something :)
        ast::node_map<span::Span> nodeSpanMap;
    };

    struct Session {
        SourceMap sourceMap;
        Option<resolve::module_ptr> modTreeRoot{None};
        NodeStorage nodeStorage;
        resolve::DefStorage defStorage;
        resolve::ResStorage resStorage;
    };
}

#endif // JACY_SESSION_H
