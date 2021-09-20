#ifndef JACY_RESOLVE_DEFTABLE_H
#define JACY_RESOLVE_DEFTABLE_H

#include "resolve/Module.h"

namespace jc::resolve {
    struct FosRedefs {
        std::vector<span::Symbol> suffixes;

        bool ok() const {
            return suffixes.empty();
        }
    };

    struct DefTable {
        auto size() const {
            return defs.size();
        }

        const auto & getDefinitions() const {
            return defs;
        }

        // Modules //
    public:
        Module::Ptr getModule(const DefId & defId) const;
        Module::Ptr getBlock(NodeId nodeId) const;
        Module::Ptr getFuncModule(FOSId overloadId, span::Symbol suffix) const;

        Module::Ptr addModule(const DefId & defId, Module::Ptr module);
        Module::Ptr addBlock(NodeId nodeId, Module::Ptr module);

        const auto & getBlocks() const {
            return blocks;
        }

        // Common definitions //
    public:
        Def getDef(const DefIndex & index) const;
        Def getDef(const DefId & defId) const;
        Vis getDefVis(const DefId & defId) const;
        const NodeId & getNodeIdByDefId(const DefId & defId) const;
        const DefId & getDefIdByNodeId(const NodeId & nodeId) const;
        span::Span getDefNameSpan(const DefId & defId) const;

        DefId define(Vis vis, NodeId nodeId, DefKind kind, const span::Ident & ident);

        // Function overloading //
    public:
        using FOSMap = std::map<Symbol, DefId>;
        using FOSList = std::vector<FOSMap>;

        const FOSList & getFOSList() const;
        const FOSMap & getFOS(FOSId fosId) const;

        FOSId newEmptyFOS();
        FOSId defineFunc(DefId defId, FOSId::Opt fosId, Symbol suffix);

        /**
         * @brief Get defId of the first overload from some function overload set
         */
        DefId getFOSFirstDef(FOSId fosId) const;

        /**
         * @brief Get span of the first overload from some function overload set
         */
        span::Span getFOSFirstSpan(FOSId fosId) const;

    private:
        FOSMap & getFOSmut(FOSId fosId) {
            return utils::arr::expectAtMut(fosList, fosId.val, "`DefTable::getFOS` mutable");
        }

        // Importation //
    public:
        DefId defineImportAlias(Vis importVis, NodeId pathNodeId, DefId importDefId);
        void setUseDeclModule(NodeId nodeId, Module::Ptr module);
        Module::Ptr getUseDeclModule(NodeId nodeId) const;
        FosRedefs importFos(Vis importVis, NodeId pathNodeId, FOSId importFosId, FOSId targetFosId);
        DefId getImportAlias(DefId aliasDefId) const;

        const auto & getUseDeclModules() const {
            return useDeclModules;
        }

        // Internal API //
    private:
        DefId nextDefId() {
            return DefId {DefIndex {defs.size()}};
        }

        DefId addDef(DefKind kind, const span::Ident & ident) {
            auto defId = nextDefId();
            defs.emplace_back(defId, kind, ident);
            return defId;
        }

        /**
         * @brief Unwinds definition in case if it is an alias
         *  Note: Always use `unwindDefId` if you want to get "real" def id
         * @param defId
         * @return
         */
        DefId unwindDefId(DefId defId) const {
            auto unwound = defId;
            while (defs.at(unwound.getIndex().val).kind == DefKind::ImportAlias) {
                unwound = getImportAlias(unwound);
            }
            return unwound;
        }

    private:
        std::vector<Def> defs;
        std::map<DefIndex, Module::Ptr> modules;
        std::map<NodeId, Module::Ptr> blocks;
        std::map<NodeId, Module::Ptr> useDeclModules;
        std::map<DefId, Vis> defVisMap;
        std::map<NodeId, DefId> nodeIdDefIdMap;
        std::map<DefId, NodeId> defIdNodeIdMap;
        std::map<DefId, DefId> importAliases;

        /// Function overload sets collection, each id points to mapping from suffix to function definition
        FOSList fosList;

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
