#include "resolve/DefTable.h"

namespace jc::resolve {
    // Modules //
    const Module::Ptr & DefTable::getModule(const DefId & defId) const {
        try {
            return modules.at(defId.getIndex());
        } catch (const std::out_of_range & e) {
            panicWithDump("Called `DefTable::getModule` with non-existing `defId` ", defId, ": ", e.what());
        }
    }

    const Module::Ptr & DefTable::getBlock(ast::NodeId nodeId) const {
        try {
            return blocks.at(nodeId);
        } catch (const std::out_of_range & e) {
            panicWithDump("Called `DefTable::getBlock` with non-existing `nodeId` ", nodeId, ": ", e.what());
        }
    }

    const Module::Ptr & DefTable::getFuncModule(FuncOverloadId overloadId, span::Symbol suffix) const {
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

    Module::Ptr DefTable::addModule(const DefId & defId, Module::Ptr module) {
        const auto & added = modules.emplace(defId.getIndex(), module);
        if (not added.second) {
            log::devPanic("[DefStorage]: Tried to add module with same defId twice");
        }
        return added.first->second;
    }

    Module::Ptr DefTable::addBlock(ast::NodeId nodeId, Module::Ptr module) {
        const auto & added = blocks.emplace(nodeId, module);
        if (not added.second) {
            log::devPanic("[DefStorage]: Tried to add block with same nodeId twice");
        }
        return added.first->second;
    }

    void DefTable::setUseDeclModule(ast::NodeId nodeId, Module::Ptr module) {
        useDeclModules.emplace(nodeId, module);
    }

    const Module::Ptr & DefTable::getUseDeclModule(ast::NodeId nodeId) const {
        try {
            return useDeclModules.at(nodeId);
        } catch (const std::out_of_range & e) {
            panicWithDump(
                "Called `DefTable::getUseDeclModule` with non-existing `nodeId` ", nodeId, ": ", e.what());
        }
    }

    // Common definitions //
    const Def & DefTable::getDef(const DefIndex & index) const {
        try {
            return defs.at(index.val);
        } catch (const std::out_of_range & e) {
            log::devPanic("Called `DefStorage::getDef` with non-existent `defId`");
        }
    }

    const Def & DefTable::getDef(const DefId & defId) const {
        return getDef(defId.getIndex());
    }

    DefVis DefTable::getDefVis(const DefId & defId) const {
        return utils::map::expectAt(defVisMap, defId, "`DefTable::getDefVis`");
    }

    const NodeId & DefTable::getNodeIdByDefId(const DefId & defId) const {
        return utils::map::expectAt(defIdNodeIdMap, defId, "`DefTable::getNodeIdByDefId`");
    }

    const DefId & DefTable::getDefIdByNodeId(const NodeId & nodeId) const {
        return utils::map::expectAt(nodeIdDefIdMap, nodeId, "`DefTable::getDefIdByNodeId`");
    }

    span::Span DefTable::getDefNameSpan(const DefId & defId) const {
        try {
            return defs.at(defId.getIndex().val).ident.span;
        } catch (const std::out_of_range & e) {
            panicWithDump("Called `DefTable::getDefNameSpan` with non-existing `defId` ", defId, ": ", e.what());
        }
    }

    DefId DefTable::define(DefVis vis, NodeId nodeId, DefKind kind, const span::Ident & ident) {
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

    // Function overloading //
    const DefTable::FuncOverloadMap & DefTable::getFuncOverload(FuncOverloadId overloadId) const {
        return utils::arr::expectAt(funcOverloads, overloadId.val, "`DefTable::getFuncOverload`");
    }

    FuncOverloadId DefTable::defineFuncOverload(DefId defId, FuncOverloadId::Opt funcOverloadId, Symbol suffix) {
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

        return funcOverloadId.unwrap();
    }

    DefId DefTable::getFuncOverloadFirstDef(FuncOverloadId funcOverloadId) const {
        return getFuncOverload(funcOverloadId).begin()->second;
    }

    span::Span DefTable::getFuncOverloadFirstSpan(FuncOverloadId funcOverloadId) const {
        return getDefNameSpan(getFuncOverloadFirstDef(funcOverloadId));
    }
}
