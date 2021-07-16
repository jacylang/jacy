#ifndef JACY_RESOLVE_DEFSTORAGE_H
#define JACY_RESOLVE_DEFSTORAGE_H

#include "resolve/Module.h"

namespace jc::resolve {
    struct DefStorage {
        const Def & getDef(const DefId & defId) const {
            try {
                return defs.at(defId.getIndex());
            } catch (std::out_of_range & e) {
                common::Logger::devPanic("Called `DefStorage::getDef` with non-existent `defId`");
            }
        }

        DefVis getDefVis(const DefId & defId) const {
            return getDef(defId).vis;
        }

        template<class ...Args>
        DefId define(Args ...args) {
            defs.emplace_back(Def(args...));
            return DefId(defs.size() - 1);
        }

        const std::vector<Def> & getDefinitions() const {
            return defs;
        }

        module_ptr addModule(const DefId & defId, module_ptr module) {
            const auto & added = modules.emplace(defId.getIndex(), module);
            if (not added.second) {
                common::Logger::devPanic("[DefStorage]: Tried to add module with same defId twice");
            }
            return added.first->second;
        }

        module_ptr addBlock(ast::node_id nodeId, module_ptr module) {
            const auto & added = blocks.emplace(nodeId, module);
            if (not added.second) {
                common::Logger::devPanic("[DefStorage]: Tried to add block with same nodeId twice");
            }
            return added.first->second;
        }

        const module_ptr & getModule(const DefId & defId) const {
            return modules.at(defId.getIndex());
        }

        const module_ptr & getBlock(ast::node_id nodeId) const {
            return blocks.at(nodeId);
        }

        void setUseDeclModule(ast::node_id nodeId, module_ptr module) {
            useDeclModules.emplace(nodeId, module);
        }

        const module_ptr & getUseDeclModule(ast::node_id nodeId) const {
            return useDeclModules.at(nodeId);
        }

    private:
        std::vector<Def> defs;
        std::map<DefIndex, module_ptr> modules;
        std::map<ast::node_id, module_ptr> blocks;
        std::map<ast::node_id, module_ptr> useDeclModules;
    };
}

#endif // JACY_RESOLVE_DEFSTORAGE_H
