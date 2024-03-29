#ifndef JACY_RESOLVE_DEFTABLE_H
#define JACY_RESOLVE_DEFTABLE_H

#include "resolve/Module.h"

namespace jc::resolve {
    using dt::Result;

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

        // Common definitions //
    public:
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

        Def getDef(const DefIndex & index) const;
        Def getDef(const DefId & defId) const;
        Def getDefUnwind(const DefId & defId) const;
        Vis getDefVis(const DefId & defId) const;
        const NodeId & getNodeIdByDefId(const DefId & defId) const;
        const DefId & getDefIdByNodeId(const NodeId & nodeId) const;
        span::Span getDefNameSpan(const DefId & defId) const;

        DefId define(Vis vis, NodeId nodeId, DefKind kind, const span::Ident & ident);

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

        // Function overloading //
    public:
        using FOSMap = std::map<Symbol, DefId>;
        using FOSList = std::vector<FOSMap>;
        using FuncDefResult = Result<FOSId, std::pair<FOSId, DefId>>;

        const FOSList & getFOSList() const;
        const FOSMap & getFOS(FOSId fosId) const;

        FOSId newEmptyFOS();
        FuncDefResult tryDefineFunc(DefId defId, FOSId::Opt fosId, Symbol suffix);

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

    private:
        std::vector<Def> defs;
        Def::DefMap<Module::Ptr> modules;
        NodeId::NodeMap<Module::Ptr> blocks;
        NodeId::NodeMap<Module::Ptr> useDeclModules;
        Def::DefMap<Vis> defVisMap;
        NodeId::NodeMap<DefId> nodeIdDefIdMap;
        Def::DefMap<NodeId> defIdNodeIdMap;
        Def::DefMap<DefId> importAliases;

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
