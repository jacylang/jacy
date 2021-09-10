#ifndef JACY_RESOLVE_MODULE_H
#define JACY_RESOLVE_MODULE_H

#include "ast/Party.h"
#include "resolve/Definition.h"
#include "resolve/Resolutions.h"

namespace jc::resolve {
    using ast::NodeId;
    using span::Symbol;

    enum class ModuleKind {
        Block,
        Def,
    };

    struct FuncOverloadId {
        using ValueT = uint16_t;

        ValueT val;

        bool operator==(const FuncOverloadId & other) const {
            return val == other.val;
        }

        bool operator<(const FuncOverloadId & other) const {
            return val < other.val;
        }
    };

    /// Definition stored in `Module`
    struct IntraModuleDef {
        using Opt = Option<IntraModuleDef>;
        using ValueT = std::variant<DefId, FuncOverloadId>;

        enum class Kind {
            /// Target definition, does not depend on additional info
            Target,
            /// Function overloading, points to function name in `DefTable::funcOverloads`
            FuncOverload,
        } kind;

        IntraModuleDef(DefId defId) : val {defId} {}
        IntraModuleDef(FuncOverloadId funcOverloadId) : val {funcOverloadId} {}

        ValueT val;
    };

    struct Module {
        using Ptr = std::shared_ptr<Module>;
        using OptPtr = Option<Ptr>;
        using NSMap = std::map<Symbol, IntraModuleDef>;
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
                log::devPanic(
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

        IntraModuleDef::Opt find(Namespace nsKind, const Symbol & name) const {
            const auto & ns = getNS(nsKind);
            const auto & def = ns.find(name);
            if (def == ns.end()) {
                return None;
            }
            return def->second;
        }

        // Search for name in all namespaces
        // Also used to find alternatives for failed resolutions
        PerNS<IntraModuleDef::Opt> findAll(const Symbol & name) const {
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
            log::notImplemented("getNS");
        }

        NSMap & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return perNS.value;
                case Namespace::Type: return perNS.type;
                case Namespace::Lifetime: return perNS.lifetime;
            }
            log::notImplemented("getNS");
        }

        template<class T>
        IntraModuleDef::Opt tryDefine(Namespace ns, const Symbol & name, const T & val) {
            const auto & defined = getNS(ns).emplace(name, IntraModuleDef {val});
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

        static inline span::Ident getInitName(const ast::Init & init) {
            return getFuncName("init", init.sig, init.span);
        }

        static inline Symbol getImplName(const ast::Node & node) {
            return span::Interner::getInstance().intern("%impl_" + std::to_string(node.id.val));
        }

        static inline span::Ident getFuncName(
            const std::string & baseName,
            const ast::FuncSig & sig,
            const span::Span & span
        ) {
            std::string name = baseName + "(";
            std::vector<Symbol> labels;
            for (const auto & param : sig.params) {
                if (param.label.some()) {
                    labels.emplace_back(param.label.unwrap().unwrap().sym);
                } else {
                    name += "_:";
                }
            }
            name += ")";
            return span::Ident {Symbol::intern(name), span};
        }
    };
}

#endif // JACY_RESOLVE_MODULE_H
