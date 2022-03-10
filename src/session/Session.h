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
#include "typeck/TypeContext.h"

namespace jc::sess {
    class NodeStorage {
    public:
        template<class T>
        ast::NodeId addNode(T & node) {
            node.id = nextNodeId();
            nodeSpanMap.emplace(node.id, node.span);
            return node.id;
        }

        template<class T>
        ast::NodeId addNode(ast::N<T> & node) {
            node->id = nextNodeId();
            nodeSpanMap.emplace(node->id, node->span);
            return node->id;
        }

        template<class T>
        ast::NodeId addNodeLike(T & nodeLike, span::Span span) {
            nodeLike.id = nextNodeId();
            nodeSpanMap.emplace(nodeLike.id, span);
            return nodeLike.id;
        }

        ast::NodeId nextNodeId() {
            return curNodeId++;
        }

        span::Span getNodeSpan(ast::NodeId nodeId) const {
            return nodeSpanMap.at(nodeId);
        }

        ast::NodeId addNode(ast::Ident & ident) {
            ident.id = nextNodeId();
            nodeSpanMap.emplace(ident.id, ident.span);
            return ident.id;
        }

        ast::NodeId size() const {
            return curNodeId;
        }

    private:
        ast::NodeId curNodeId {1}; // Reserve `0` for something :)
        ast::NodeId::NodeMap<span::Span> nodeSpanMap;
    };

    struct Session {
        using Ptr = std::shared_ptr<Session>;

        Session();

        SourceMap sourceMap;
        Option<resolve::Module::Ptr> modTreeRoot = None;
        NodeStorage nodeStorage; // TODO: Maybe remove?
        resolve::DefTable defTable;
        resolve::Resolutions resolutions;
        typeck::TypeContext tyCtx;

        // TODO!: Move to separate wrapper for name resolution stage
        resolve::Def getResDef(ast::NodeId nodeId) const {
            return defTable.getDef(resolutions.getDefRes(nodeId));
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
