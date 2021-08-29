#ifndef JACY_RESOLVE_DEFSTORAGE_H
#define JACY_RESOLVE_DEFSTORAGE_H

#include "resolve/Module.h"

namespace jc::resolve {
    struct DefStorage {
        const Def & getDef(const DefIndex & index) const {
            try {
                return defs.at(index.val);
            } catch (const std::out_of_range & e) {
                log::Logger::devPanic("Called `DefStorage::getDef` with non-existent `defId`");
            }
        }

        const Def & getDef(const DefId & defId) const {
            return getDef(defId.getIndex());
        }

        DefVis getDefVis(const DefId & defId) const {
            return getDef(defId).vis;
        }

        auto size() const {
            return defs.size();
        }

        template<class ...Args>
        DefId define(Args ...args) {
            defs.emplace_back(Def(args...));
            return DefId(DefIndex {defs.size() - 1});
        }

        const std::vector<Def> & getDefinitions() const {
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

        span::Span::Opt getDefNameSpan(DefId defId) const {
            const auto & found = defNameSpans.find(defId);
            if (found != defNameSpans.end()) {
                return found->second;
            }
            return None;
        }

    private:
        std::vector<Def> defs;
        std::map<DefIndex, Module::Ptr> modules;
        std::map<ast::NodeId, Module::Ptr> blocks;
        std::map<ast::NodeId, Module::Ptr> useDeclModules;

        /// Stores names (identifiers) of definitions (if exists).
        /// Used mostly for error reporting.
        std::map<DefId, span::Span> defNameSpans;
    };
}

#endif // JACY_RESOLVE_DEFSTORAGE_H
