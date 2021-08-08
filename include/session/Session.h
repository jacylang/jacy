#ifndef JACY_SESSION_H
#define JACY_SESSION_H

#include <string>
#include <memory>
#include <vector>
#include <random>

#include "log/Logger.h"
#include "session/SourceMap.h"
#include "resolve/DefStorage.h"
#include "session/diagnostics.h"

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

        void addNode(ast::Ident & ident) {
            ident.id = nextNodeId++;
            nodeSpanMap.emplace(ident.id, ident.span);
        }

        ast::node_id size() const {
            return nextNodeId;
        }

    private:
        ast::node_id nextNodeId = 1; // Reserve `0` for something :)
        ast::node_map<span::Span> nodeSpanMap;
    };

    struct Session {
        Session();

        SourceMap sourceMap;
        Option<resolve::module_ptr> modTreeRoot{None};
        NodeStorage nodeStorage;
        resolve::DefStorage defStorage;
        resolve::ResStorage resStorage;

        // TODO!: Move to separate wrapper for name resolution stage
        resolve::Def getResDef(ast::node_id nodeId) const {
            return defStorage.getDef(resStorage.getDefRes(nodeId));
        }

        // Diagnostics //
    public:
        Step::ptr step;
        void beginStep(const std::string & name, MeasUnit measUnit);
        void endStep(Option<size_t> procUnitCount = None);

        void printSteps() noexcept;
        void printStepsDevMode();
    };
}

#endif // JACY_SESSION_H
