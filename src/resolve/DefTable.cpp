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

    const Module::Ptr & DefTable::getFuncModule(FOSId overloadId, span::Symbol suffix) const {
        try {
            return getModule(fosList.at(overloadId.val).at(suffix));
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

    Vis DefTable::getDefVis(const DefId & defId) const {
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

    DefId DefTable::define(Vis vis, NodeId nodeId, DefKind kind, const span::Ident & ident) {
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

    // Function overload sets //
    const DefTable::FOSList & DefTable::getFOSList() const {
        return fosList;
    }

    const DefTable::FOSMap & DefTable::getFOS(FOSId fosId) const {
        return utils::arr::expectAt(fosList, fosId.val, "`DefTable::getFOS`");
    }

    /**
     * @brief Defines function in given FOS, if no FOSId given creates one
     * @param defId
     * @param fosId
     * @param suffix
     * @return
     */
    FOSId DefTable::defineFunc(DefId defId, FOSId::Opt fosId, Symbol suffix) {
        using namespace utils::map;

        // Add new overloading indexing if not provided
        if (fosId.none()) {
            fosId = FOSId {static_cast<FOSId::ValueT>(fosList.size())};
            fosList.emplace_back();
        }

        auto & fos = utils::arr::expectAtMut(
            fosList, fosId.unwrap().val, "`DefTable::defineFunc`"
        );

        assertNewEmplace(fos.emplace(suffix, defId), "`DefTable::defineFunc` -> `overload`");

        return fosId.unwrap();
    }

    DefId DefTable::getFOSFirstDef(FOSId fosId) const {
        return getFOS(fosId).begin()->second;
    }

    span::Span DefTable::getFOSFirstSpan(FOSId fosId) const {
        return getDefNameSpan(getFOSFirstDef(fosId));
    }
}
