#include "resolve/DefTable.h"

namespace jc::resolve {
    // Common definitions //

    /// Returns definition, considers that definition exists.
    /// For `DefKind::Import` definitions, applies logic of recursive unwinding
    Def DefTable::getDef(const DefIndex & index) const {
        try {
            return defs.at(index.val);
        } catch (const std::out_of_range & e) {
            log::devPanic("Called `DefStorage::getDef` with non-existent `defId`");
        }
    }

    Def DefTable::getDef(const DefId & defId) const {
        return getDef(defId.getIndex());
    }

    Def DefTable::getDefUnwind(const DefId & defId) const {
        return getDef(unwindDefId(defId));
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
            return getDef(defId).ident.span;
        } catch (const std::out_of_range & e) {
            panicWithDump("Called `DefTable::getDefNameSpan` with non-existing `defId` ", defId, ": ", e.what());
        }
    }

    DefId DefTable::define(Vis vis, NodeId nodeId, DefKind kind, const span::Ident & ident) {
        using namespace utils::map;

        auto defId = addDef(kind, ident);

        log::Logger::devDebug(
            "[DefTable::define] Add definition ",
            Def::visStr(vis),
            Def::kindStr(kind),
            " '",
            ident,
            "' with node id ",
            nodeId,
            " and def id ",
            defId);

        assertNewEmplace(defVisMap.emplace(defId, vis), "`DefTable::define` -> defVisMap");
        assertNewEmplace(nodeIdDefIdMap.emplace(nodeId, defId), "`DefTable::define` -> nodeIdDefIdMap");
        assertNewEmplace(defIdNodeIdMap.emplace(defId, nodeId), "`DefTable::define` -> defIdNodeIdMap");

        return defId;
    }

    // Modules //
    /**
     * @brief
     * @remark Unwinds DefId (!)
     * @param defId
     * @return
     */
    Module::Ptr DefTable::getModule(const DefId & defId) const {
        try {
            return modules.at(unwindDefId(defId));
        } catch (const std::out_of_range & e) {
            panicWithDump("Called `DefTable::getModule` with non-existing `defId` ", defId, ": ", e.what());
        }
    }

    Module::Ptr DefTable::getBlock(NodeId nodeId) const {
        try {
            return blocks.at(nodeId);
        } catch (const std::out_of_range & e) {
            panicWithDump("Called `DefTable::getBlock` with non-existing `nodeId` ", nodeId, ": ", e.what());
        }
    }

    Module::Ptr DefTable::getFuncModule(FOSId overloadId, span::Symbol suffix) const {
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

    Module::Ptr DefTable::addBlock(NodeId nodeId, Module::Ptr module) {
        const auto & added = blocks.emplace(nodeId, module);
        if (not added.second) {
            log::devPanic("[DefStorage]: Tried to add block with same nodeId twice");
        }
        return added.first->second;
    }

    // Function overload sets //
    const DefTable::FOSList & DefTable::getFOSList() const {
        return fosList;
    }

    const DefTable::FOSMap & DefTable::getFOS(FOSId fosId) const {
        return utils::arr::expectAt(fosList, fosId.val, "`DefTable::getFOS`");
    }

    FOSId DefTable::newEmptyFOS() {
        fosList.emplace_back();
        return FOSId {static_cast<FOSId::ValueT>(fosList.size() - 1)};
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
            fosId = newEmptyFOS();
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

    // Importation //
    DefId DefTable::defineImportAlias(Vis importVis, NodeId pathNodeId, DefId importDefId) {
        using namespace utils::map;

        const auto & importDef = getDef(importDefId);
        auto importDefIdent = importDef.ident;
        auto aliasDefId = addDef(DefKind::ImportAlias, importDefIdent);

        log::Logger::devDebug(
            "[DefTable::define] Add import alias definition ",
            Def::visStr(importVis),
            aliasDefId,
            " '",
            importDefIdent,
            "', alias to ",
            importDefId);

        // Note: Don't define `NodeId -> DefId` mapping,
        //  because `use ...::*` can have multiple non-unique bindings, thus we will have an error

        assertNewEmplace(defVisMap.emplace(aliasDefId, importVis), "`DefTable::defineImportAlias` -> defVisMap");
        assertNewEmplace(
            defIdNodeIdMap.emplace(aliasDefId, pathNodeId),
            "`DefTable::defineImportAlias` -> defIdNodeIdMap"
        );
        assertNewEmplace(
            importAliases.emplace(aliasDefId, importDefId),
            "`DefTable::defineImportAlias` -> importAliases"
        );

        return aliasDefId;
    }

    void DefTable::setUseDeclModule(NodeId nodeId, Module::Ptr module) {
        useDeclModules.emplace(nodeId, module);
    }

    Module::Ptr DefTable::getUseDeclModule(NodeId nodeId) const {
        try {
            return useDeclModules.at(nodeId);
        } catch (const std::out_of_range & e) {
            panicWithDump(
                "Called `DefTable::getUseDeclModule` with non-existing `nodeId` ", nodeId, ": ", e.what());
        }
    }

    FosRedefs DefTable::importFos(Vis importVis, NodeId pathNodeId, FOSId importFosId, FOSId targetFosId) {
        FosRedefs redefs;

        // Get FOS we will import overloads to
        auto & targetFos = getFOSmut(targetFosId);

        log::Logger::devDebug(
            "`DefTable::importFos`: ",
            Def::visStr(importVis),
            "import ",
            importFosId,
            " to ",
            targetFosId
        );

        // Go through all overloads in imported FOS
        for (const auto & overload : getFOS(importFosId)) {
            auto overloadVis = getDefVis(overload.second);
            if (overloadVis != Vis::Pub) {
                log::Logger::devDebug(
                    "`DefTable::importFos`: ignore overload '",
                    overload.first,
                    "'",
                    overload.second,
                    " as private"
                );
                continue;
            }

            if (utils::map::has(targetFos, overload.first)) {
                log::Logger::devDebug(
                    "`DefTable::importFos`: Tried to redefine overload '",
                    overload.first,
                    "'",
                    overload.second
                );

                // If imported suffix already defined in target FOS, it is an error,
                //  however only if we imported public overload
                redefs.suffixes.emplace_back(overload.first);
            } else {
                log::Logger::devDebug(
                    "`DefTable::importFos`: Add overload '",
                    overload.first,
                    "'",
                    overload.second,
                    " from ",
                    importFosId,
                    " to ",
                    targetFosId
                );

                // Import particular overload to target FOS
                targetFos.emplace(
                    overload.first,
                    defineImportAlias(importVis, pathNodeId, overload.second)
                );
            }
        }

        return redefs;
    }

    DefId DefTable::getImportAlias(DefId aliasDefId) const {
        return importAliases.at(aliasDefId);
    }
}
