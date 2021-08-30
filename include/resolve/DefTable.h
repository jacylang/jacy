#ifndef JACY_RESOLVE_DEFTABLE_H
#define JACY_RESOLVE_DEFTABLE_H

#include "resolve/Module.h"

namespace jc::resolve {
    struct DefTable {
        const auto & getDef(const DefIndex & index) const {
            try {
                return defs.at(index.val);
            } catch (const std::out_of_range & e) {
                log::Logger::devPanic("Called `DefStorage::getDef` with non-existent `defId`");
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

        auto size() const {
            return defs.size();
        }

        DefId define(DefVis vis, NodeId nodeId, DefKind kind, const span::Ident & ident) {
            using namespace utils::map;

            auto defId = DefId{defs.size() - 1};
            defs.emplace_back(Def {defId, kind, ident});

            assertNewEmplace(defVisMap.emplace(defId, vis), "`DefTable::addDef` -> defVisMap");
            assertNewEmplace(nodeIdDefIdMap.emplace(nodeId, defId), "`DefTable::addDef` -> nodeIdDefIdMap");
            assertNewEmplace(defIdNodeIdMap.emplace(defId, nodeId), "`DefTable::addDef` -> defIdNodeIdMap");

            return defId;
        }

        const auto & getDefinitions() const {
            return defs;
        }

        Module::Ptr addModule(const DefId & defId, Module::Ptr module) {
            const auto & added = modules.emplace(defId.getIndex(), module);
            if (not added.second) {
                log::Logger::devPanic("[DefStorage]: Tried to add module with same defId twice");
            }
            return added.first->second;
        }

        Module::Ptr addBlock(ast::NodeId nodeId, Module::Ptr module) {
            const auto & added = blocks.emplace(nodeId, module);
            if (not added.second) {
                log::Logger::devPanic("[DefStorage]: Tried to add block with same nodeId twice");
            }
            return added.first->second;
        }

        const Module::Ptr & getModule(const DefId & defId) const {
            try {
                return modules.at(defId.getIndex());
            } catch (const std::out_of_range & e) {
                log::Logger::devPanic("Called `DefStorage::getModule` with non-existent `defId` ", defId);
            }
        }

        const Module::Ptr & getBlock(ast::NodeId nodeId) const {
            return blocks.at(nodeId);
        }

        void setUseDeclModule(ast::NodeId nodeId, Module::Ptr module) {
            useDeclModules.emplace(nodeId, module);
        }

        const Module::Ptr & getUseDeclModule(ast::NodeId nodeId) const {
            return useDeclModules.at(nodeId);
        }

        span::Span::Opt getDefNameSpan(const DefId & defId) const {
            return defs.at(defId.getIndex().val).ident.span;
        }

    private:
        std::vector<Def> defs;
        std::map<DefIndex, Module::Ptr> modules;
        std::map<ast::NodeId, Module::Ptr> blocks;
        std::map<ast::NodeId, Module::Ptr> useDeclModules;
        std::map<DefId, DefVis> defVisMap;
        std::map<ast::NodeId, DefId> nodeIdDefIdMap;
        std::map<DefId, ast::NodeId> defIdNodeIdMap;

        /// Stores names (identifiers) of definitions (if exists).
        /// Used mostly for error reporting.
//        std::map<DefId, span::Span> defNameSpans;
    };
}

#endif // JACY_RESOLVE_DEFTABLE_H
