#ifndef JACY_SESSION_H
#define JACY_SESSION_H

#include <string>
#include <memory>
#include <vector>
#include <random>

#include "log/Logger.h"
#include "session/SourceMap.h"
#include "resolve/DefTable.h"
#include "session/diagnostics.h"

namespace jc::sess {
    class NodeStorage {
    public:
        template<class T>
        void addNode(T & node) {
            node.id.val = nextNodeId.val++;
            nodeSpanMap.emplace(node.id, node.span);
        }

        template<class T>
        void addNode(ast::N<T> & node) {
            node->id.val = nextNodeId.val++;
            nodeSpanMap.emplace(node->id, node->span);
        }

        const span::Span & getNodeSpan(ast::NodeId nodeId) const {
            return nodeSpanMap.at(nodeId);
        }

        void addNode(ast::Ident & ident) {
            ident.id.val = nextNodeId.val++;
            nodeSpanMap.emplace(ident.id, ident.span);
        }

        ast::NodeId size() const {
            return nextNodeId;
        }

    private:
        ast::NodeId nextNodeId{1}; // Reserve `0` for something :)
        ast::NodeId::NodeMap<span::Span> nodeSpanMap;
    };

    struct Session {
        using Ptr = std::shared_ptr<Session>;

        Session();

        SourceMap sourceMap;
        Option<resolve::Module::Ptr> modTreeRoot{None};
        NodeStorage nodeStorage;
        resolve::DefTable defStorage;
        resolve::ResStorage resStorage;

        // TODO!: Move to separate wrapper for name resolution stage
        resolve::Def getResDef(ast::NodeId nodeId) const {
            return defStorage.getDef(resStorage.getDefRes(nodeId));
        }

        // Diagnostics //
    public:
        Step::Ptr step;
        void beginStep(const std::string & name, MeasUnit measUnit);
        void endStep(Option<size_t> procUnitCount = None);

        void printSteps();
        void printStepsDevMode();
    };
}

#endif // JACY_SESSION_H
