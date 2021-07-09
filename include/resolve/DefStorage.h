#ifndef JACY_RESOLVE_DEFSTORAGE_H
#define JACY_RESOLVE_DEFSTORAGE_H

#include "resolve/Module.h"

namespace jc::resolve {
    struct DefStorage {
        const Def & getDef(def_id defId) const {
            try {
                return defs.at(defId);
            } catch (std::out_of_range & e) {
                common::Logger::devPanic("Called `DefStorage::getDef` with non-existent `defId`");
            }
        }

        DefVis isPrivateFor(mod_depth modDepth) const {
        }

        template<class ...Args>
        def_id define(Args ...args) {
            defs.emplace_back(Def(args...));
            return defs.size() - 1;
        }

        const std::vector<Def> & getDefinitions() const {
            return defs;
        }

        module_ptr addModule(def_id defId, module_ptr module) {
            const auto & added = modules.emplace(defId, module);
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

        const module_ptr & getModule(def_id defId) const {
            return modules.at(defId);
        }

        const module_ptr & getBlock(ast::node_id nodeId) const {
            return blocks.at(nodeId);
        }

    private:
        std::vector<Def> defs;
        std::map<def_id, module_ptr> modules;
        std::map<ast::node_id, module_ptr> blocks;
    };
}

#endif // JACY_RESOLVE_DEFSTORAGE_H
