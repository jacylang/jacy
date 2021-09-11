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
        using Opt = Option<FuncOverloadId>;
        using ValueT = uint16_t;

        FuncOverloadId(ValueT val) : val {val} {}

        ValueT val;

        bool operator==(const FuncOverloadId & other) const {
            return val == other.val;
        }

        bool operator<(const FuncOverloadId & other) const {
            return val < other.val;
        }

        friend std::ostream & operator<<(std::ostream & os, const FuncOverloadId & overloadId) {
            return os << log::Color::Magenta << "#fo(" << overloadId.val << ")" << log::Color::Reset;
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

        auto asDef() const {
            return std::get<DefId>(val);
        }

        auto asFuncOverload() const {
            return std::get<FuncOverloadId>(val);
        }

        friend std::ostream & operator<<(std::ostream & os, const IntraModuleDef & intraModuleDef) {
            if (intraModuleDef.kind == IntraModuleDef::Kind::Target) {
                return os << log::Color::Magenta << intraModuleDef.asDef() << log::Color::Reset;
            } else if (intraModuleDef.kind == IntraModuleDef::Kind::FuncOverload) {
                return os << log::Color::Magenta << intraModuleDef.asFuncOverload() << log::Color::Reset;
            }
            log::devPanic("Unhandled `IntraModuleDef::Kind` in `operator<<`");
        }
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

        // Constructors //
    public:
        // `Block` module
        static inline Ptr newBlockModule(NodeId nodeId, Ptr parent, DefId::Opt nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Block, parent, nodeId, nearestModDef);
        }

        // `Def` module
        static inline Ptr newDefModule(const DefId & defId, Ptr parent, DefId::Opt nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Def, parent, defId, nearestModDef);
        }

    public:
        void assertKind(ModuleKind kind) const;
        auto getNodeId() const;
        const auto & getDefId() const;
        const NSMap & getNS(Namespace ns) const;
        NSMap & getNS(Namespace ns);

        IntraModuleDef::Opt find(Namespace nsKind, const Symbol & name) const;
        PerNS<IntraModuleDef::Opt> findAll(const Symbol & name) const;

        template<class T>
        IntraModuleDef::Opt tryDefine(Namespace ns, const Symbol & name, const T & val) {
            const auto & defined = getNS(ns).emplace(name, IntraModuleDef {val});
            // Note: emplace returns `pair<new element iterator, true>` if new element added
            //  and `pair<old element iterator, false>` if tried to re-emplace
            if (not defined.second) {
                // Return old def id
                return defined.first->second;
            }
            return None;
        }

    public:
        static inline Symbol getInitName(const ast::Init & init) {
            /// TODO: Use real `init` span
            return getFuncSuffix(init.sig, init.span);
        }

        static inline Symbol getImplName(const ast::Node & node) {
            return span::Interner::getInstance().intern("%impl_" + std::to_string(node.id.val));
        }

        // Get suffix of form like `(label1:label2:_:...)`
        static inline Symbol getFuncSuffix(
            const ast::FuncSig & sig,
            const span::Span & span
        ) {
            std::string name = "(";
            std::vector<Symbol> labels;
            for (const auto & param : sig.params) {
                if (param.label.some()) {
                    labels.emplace_back(param.label.unwrap().unwrap().sym);
                } else {
                    labels.emplace_back(Symbol::fromKw(span::Kw::Underscore));
                }
            }

            std::sort(labels.begin(), labels.end(), [](const Symbol & lhs, const Symbol & rhs) {
                return lhs < rhs;
            });

            name += ")";
            return Symbol::intern(name);
        }

        // Representation //
    public:
        std::string toString() const;

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
    };
}

#endif // JACY_RESOLVE_MODULE_H
