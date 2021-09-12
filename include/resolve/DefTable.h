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

        const auto & getFuncOverload(FuncOverloadId overloadId) const {
            return utils::arr::expectAt(funcOverloads, overloadId.val, "`DefTable::getFuncOverload`");
        }

        DefVis getDefVis(const DefId & defId) const {
            return utils::map::expectAt(defVisMap, defId, "`DefTable::getDefVis`");
        }

        const auto & getNodeIdByDefId(const DefId & defId) const {
            return utils::map::expectAt(defIdNodeIdMap, defId, "`DefTable::getNodeIdByDefId`");
        }

        const auto & getDefIdByNodeId(const NodeId & nodeId) const {
            return utils::map::expectAt(nodeIdDefIdMap, nodeId, "`DefTable::getDefIdByNodeId`");
        }

        auto size() const {
            return defs.size();
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
                panicWithDump("Called `DefTable::getModule` with non-existing `defId` ", defId, ": ", e.what());
            }
        }

        const Module::Ptr & getBlock(ast::NodeId nodeId) const {
            try {
                return blocks.at(nodeId);
            } catch (const std::out_of_range & e) {
                panicWithDump("Called `DefTable::getBlock` with non-existing `nodeId` ", nodeId, ": ", e.what());
            }
        }

        const Module::Ptr & getFuncModule(FuncOverloadId overloadId, span::Symbol suffix) const {
            try {
                return getModule(funcOverloads.at(overloadId.val).at(suffix));
            } catch (const std::out_of_range & e) {
                panicWithDump(
                    "Called `DefTable::getFuncModule` with non-existing FuncOverloadId '",
                    overloadId,
                    "' or suffix '",
                    suffix,
                    "'"
                );
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
                    "Called `DefTable::getUseDeclModule` with non-existing `nodeId` ", nodeId, ": ", e.what());
            }
        }

        span::Span getDefNameSpan(const DefId & defId) const {
            try {
                return defs.at(defId.getIndex().val).ident.span;
            } catch (const std::out_of_range & e) {
                panicWithDump("Called `DefTable::getDefNameSpan` with non-existing `defId` ", defId, ": ", e.what());
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

    public:
        DefId define(DefVis vis, NodeId nodeId, DefKind kind, const span::Ident & ident) {
            return defineCommon(vis, nodeId, kind, ident);
        }

        // Function overloading //
    public:
        using FuncOverloadMap = std::map<Symbol, DefId>;

        void defineFuncOverload(
            DefId defId,
            FuncOverloadId::Opt funcOverloadId,
            Symbol suffix
        ) {
            using namespace utils::map;

            // Add new overloading indexing if not provided
            if (funcOverloadId.none()) {
                funcOverloadId = FuncOverloadId {static_cast<FuncOverloadId::ValueT>(funcOverloads.size())};
                funcOverloads.emplace_back();
            }

            auto & overload = utils::arr::expectAtMut(
                funcOverloads, funcOverloadId.unwrap().val, "`DefTable::defineFuncOverload`"
            );

            assertNewEmplace(overload.emplace(suffix, defId), "`DefTable::defineFunc` -> `overload`");
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
        std::vector<FuncOverloadMap> funcOverloads;

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
