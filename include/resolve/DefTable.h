#ifndef JACY_RESOLVE_DEFTABLE_H
#define JACY_RESOLVE_DEFTABLE_H

#include "resolve/Module.h"

namespace jc::resolve {
    struct DefTable {
        const auto & getDef(const DefIndex & index) const {
            try {
                return defs.at(index.val);
            } catch (const std::out_of_range & e) {
                log::devPanic("Called `DefStorage::getDef` with non-existent `defId`");
            }
        }

        const auto & getDef(const DefId & defId) const {
            return getDef(defId.getIndex());
        }

        DefVis getDefVis(const DefId & defId) const {
            return defVisMap.at(defId);
        }

        const auto & getNodeIdByDefId(const DefId & defId) const {
            return defIdNodeIdMap.at(defId);
        }

        const auto & getDefIdByNodeId(const NodeId & nodeId) const {
            return nodeIdDefIdMap.at(nodeId);
        }

        auto size() const {
            return defs.size();
        }

        DefId define(DefVis vis, NodeId nodeId, DefKind kind, const span::Ident & ident) {
            return defineCommon(vis, nodeId, kind, ident);
        }

        DefId defineFunc(
            DefVis vis,
            NodeId nodeId,
            FuncOverloadId funcOverloadId,
            Symbol suffix,
            const span::Ident & ident
        ) {
            using namespace utils::map;

            /// Emplace overloading definition
            auto defId = defineCommon(vis, nodeId, DefKind::Func, ident);

            auto & overload = utils::arr::expectAt(funcOverloads, funcOverloadId.val, "`DefTable::defineFunc`");

            assertNewEmplace(overload.emplace(suffix, defId), "`DefTable::defineFunc` -> `overload`");

            return defId;
        }

        const auto & getDefinitions() const {
            return defs;
        }

        Module::Ptr addModule(const DefId & defId, Module::Ptr module) {
            const auto & added = modules.emplace(defId.getIndex(), module);
            if (not added.second) {
                log::devPanic("[DefStorage]: Tried to add module with same defId twice");
            }
            return added.first->second;
        }

        Module::Ptr addBlock(ast::NodeId nodeId, Module::Ptr module) {
            const auto & added = blocks.emplace(nodeId, module);
            if (not added.second) {
                log::devPanic("[DefStorage]: Tried to add block with same nodeId twice");
            }
            return added.first->second;
        }

        const Module::Ptr & getModule(const DefId & defId) const {
            try {
                return modules.at(defId.getIndex());
            } catch (const std::out_of_range & e) {
                panicWithDump("Called `DefStorage::getModule` with non-existent `defId` ", defId, ": ", e.what());
            }
        }

        const Module::Ptr & getBlock(ast::NodeId nodeId) const {
            try {
                return blocks.at(nodeId);
            } catch (const std::out_of_range & e) {
                panicWithDump("Called `DefStorage::getBlock` with non-existent `nodeId` ", nodeId, ": ", e.what());
            }
        }

        void setUseDeclModule(ast::NodeId nodeId, Module::Ptr module) {
            useDeclModules.emplace(nodeId, module);
        }

        const Module::Ptr & getUseDeclModule(ast::NodeId nodeId) const {
            try {
                return useDeclModules.at(nodeId);
            } catch (const std::out_of_range & e) {
                panicWithDump(
                    "Called `DefStorage::getUseDeclModule` with non-existent `nodeId` ", nodeId, ": ", e.what());
            }
        }

        span::Span getDefNameSpan(const DefId & defId) const {
            try {
                return defs.at(defId.getIndex().val).ident.span;
            } catch (const std::out_of_range & e) {
                panicWithDump("Called `DefStorage::getDefNameSpan` with non-existent `defId` ", defId, ": ", e.what());
            }
        }

    private:
        DefId defineCommon(DefVis vis, NodeId nodeId, DefKind kind, const span::Ident & ident) {
            using namespace utils::map;

            auto defId = DefId {DefIndex {defs.size()}};
            defs.emplace_back(defId, kind, ident);

            log::Logger::devDebug(
                "[DefTable::define] Add definition ",
                Def::kindStr(kind),
                " '",
                ident,
                "' with node id ",
                nodeId,
                " and def id ",
                defId);

            assertNewEmplace(defVisMap.emplace(defId, vis), "`DefTable::addDef` -> defVisMap");
            assertNewEmplace(nodeIdDefIdMap.emplace(nodeId, defId), "`DefTable::addDef` -> nodeIdDefIdMap");
            assertNewEmplace(defIdNodeIdMap.emplace(defId, nodeId), "`DefTable::addDef` -> defIdNodeIdMap");

            return defId;
        }

    private:
        std::vector<Def> defs;
        std::map<DefIndex, Module::Ptr> modules;
        std::map<ast::NodeId, Module::Ptr> blocks;
        std::map<ast::NodeId, Module::Ptr> useDeclModules;
        std::map<DefId, DefVis> defVisMap;
        std::map<ast::NodeId, DefId> nodeIdDefIdMap;
        std::map<DefId, ast::NodeId> defIdNodeIdMap;

        /// Function overloads, each id points to mapping from suffix to function definition
        std::vector<std::map<Symbol, DefId>> funcOverloads;

        template<class ...Args>
        void panicWithDump(Args ...args) const {
            log::devPanic(std::forward<Args>(args)..., "\nDefinitions: ", defs);
        }

        /// Stores names (identifiers) of definitions (if exists).
        /// Used mostly for error reporting.
//        std::map<DefId, span::Span> defNameSpans;
    };
}

#endif // JACY_RESOLVE_DEFTABLE_H
