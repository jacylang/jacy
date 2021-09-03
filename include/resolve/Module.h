#ifndef JACY_RESOLVE_MODULESTACK_H
#define JACY_RESOLVE_MODULESTACK_H

#include "ast/Party.h"
#include "resolve/Definition.h"
#include "resolve/Resolutions.h"

namespace jc::resolve {
    using ast::NodeId;

    enum class ModuleKind {
        Block,
        Def,
    };

    struct Module {
        using Ptr = std::shared_ptr<Module>;
        using OptPtr = Option<Ptr>;
        using NSMap = std::map<std::string, DefId>;
        using IdType = std::variant<NodeId, DefId>;

        Module(
            ModuleKind kind,
            OptPtr parent,
            IdType id,
            DefId::Opt nearestModDef
        ) : kind{kind},
            parent{parent},
            id{id},
            nearestModDef{nearestModDef} {}

        ModuleKind kind;
        OptPtr parent{None};

        // Can either be `NodeId` (Block) or `ModuleDef` (Module definition with additional info)
        IdType id;

        // Nearest `mod` definition
        DefId::Opt nearestModDef;

        PerNS<NSMap> perNS;
        PrimTypeSet shadowedPrimTypes{0};

        // `Block` module
        static inline Ptr newBlockModule(NodeId nodeId, Ptr parent, DefId::Opt nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Block, parent, nodeId, nearestModDef);
        }

        // `Def` module
        static inline Ptr newDefModule(const DefId & defId, Ptr parent, DefId::Opt nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Def, parent, defId, nearestModDef);
        }

        void assertKind(ModuleKind kind) const {
            if (this->kind != kind) {
                log::Logger::devPanic(
                    "[Module::assertKind] Failed - expected '" + kindStr(kind) + "', got '" + kindStr() + "'");
            }
        }

        auto getNodeId() const {
            assertKind(ModuleKind::Block);
            return std::get<NodeId>(id);
        }

        const auto & getDefId() const {
            assertKind(ModuleKind::Def);
            return std::get<DefId>(id);
        }

        DefId::Opt find(Namespace nsKind, const std::string & name) const {
            const auto & ns = getNS(nsKind);
            const auto & def = ns.find(name);
            if (def == ns.end()) {
                return None;
            }
            return def->second;
        }

        // Search for name in all namespaces
        // Also used to find alternatives for failed resolutions
        PerNS<DefId::Opt> findAll(const std::string & name) const {
            return {
                find(Namespace::Value, name),
                find(Namespace::Type, name),
                find(Namespace::Lifetime, name)
            };
        }

        const NSMap & getNS(Namespace ns) const {
            switch (ns) {
                case Namespace::Value: return perNS.value;
                case Namespace::Type: return perNS.type;
                case Namespace::Lifetime: return perNS.lifetime;
            }
            log::Logger::notImplemented("getNS");
        }

        NSMap & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return perNS.value;
                case Namespace::Type: return perNS.type;
                case Namespace::Lifetime: return perNS.lifetime;
            }
            log::Logger::notImplemented("getNS");
        }

        DefId::Opt tryDefine(Namespace ns, const std::string & name, const DefId & defId) {
            const auto & defined = getNS(ns).emplace(name, defId);
            // Note: emplace returns `pair<new element iterator, true>` if emplaced new element
            //  and `pair<old element iterator, false>` if tried to re-emplace
            if (not defined.second) {
                // Return old def id
                return defined.first->second;
            }
            return None;
        }

        std::string toString() const {
            std::string repr = kindStr();
            repr += " ";
            if (kind == ModuleKind::Block) {
                repr += "block #" + std::get<NodeId>(id).toString();
            } else if (kind == ModuleKind::Def) {
                repr += "module #" + std::get<DefId>(id).getIndex().toString();
            }
            return repr;
        }

        static inline std::string kindStr(ModuleKind kind) {
            switch (kind) {
                case ModuleKind::Block: return "[BLOCK]";
                case ModuleKind::Def: return "[DEF]";
                default: return "[NO REPRESENTATION (bug)]";
            }
        }

        inline std::string kindStr() const {
            return kindStr(kind);
        }

        static inline std::string nsToString(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return "value";
                case Namespace::Type: return "type";
                case Namespace::Lifetime: return "lifetime";
                default: return "[NO REPRESENTATION (bug)]";
            }
        }

        static inline std::string getInitName(const ast::Node & node) {
            return "%init_" + std::to_string(node.id.val);
        }

        static inline std::string getImplName(const ast::Node & node) {
            return "%impl_" + std::to_string(node.id.val);
        }
    };
}

#endif // JACY_RESOLVE_MODULESTACK_H
