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
    struct NameBinding {
        using Opt = Option<NameBinding>;
        using ValueT = std::variant<DefId, FuncOverloadId>;
        using PerNS = PerNS<NameBinding::Opt>;

        enum class Kind {
            /// Target definition, does not depend on additional info
            Target,

            /// Function overloading, points to function name in `DefTable::funcOverloads`
            FuncOverload,
        } kind;

        NameBinding(DefId defId) : kind{Kind::Target}, val{defId} {}
        NameBinding(FuncOverloadId funcOverloadId) : kind{Kind::FuncOverload}, val {funcOverloadId} {}

        ValueT val;

        bool isTarget() const {
            return kind == Kind::Target;
        }

        bool isFuncOverload() const {
            return kind == Kind::FuncOverload;
        }

        void assertKind(Kind expected) const {
            if (kind != expected) {
                log::devPanic(
                    "`IntraModuleDef` kind assertion failed, expected '",
                    kindStr(expected), "', got '", kindStr(kind), "'"
                );
            }
        }

        DefId asDef() const {
            assertKind(Kind::Target);
            return std::get<DefId>(val);
        }

        FuncOverloadId asFuncOverload() const {
            assertKind(Kind::FuncOverload);
            return std::get<FuncOverloadId>(val);
        }

        constexpr static inline const char * kindStr(Kind kind) {
            switch (kind) {
                case Kind::Target: return "target";
                case Kind::FuncOverload: return "function overload";
            }
        }

        friend std::ostream & operator<<(std::ostream & os, const NameBinding & intraModuleDef) {
            if (intraModuleDef.isTarget()) {
                return os << log::Color::Magenta << intraModuleDef.asDef() << log::Color::Reset;
            } else if (intraModuleDef.isFuncOverload()) {
                return os << log::Color::Magenta << intraModuleDef.asFuncOverload() << log::Color::Reset;
            }
            log::devPanic("Unhandled `IntraModuleDef::Kind` in `operator<<`");
        }
    };

    struct Module {
        using Ptr = std::shared_ptr<Module>;
        using OptPtr = Option<Ptr>;
        using NSMap = std::map<Symbol, NameBinding>;
        using IdType = std::variant<NodeId, DefId>;

        Module(
            ModuleKind kind,
            OptPtr parent,
            IdType id,
            DefId nearestModDef
        ) : kind{kind},
            parent{parent},
            id{id},
            nearestModDef{nearestModDef} {}

        ModuleKind kind;
        OptPtr parent = None;

        // Can either be `NodeId` (Block) or `DefId` (Module definition)
        IdType id;

        // Nearest `mod` definition
        DefId nearestModDef;

        PerNS<NSMap> perNS;
        PrimTypeSet shadowedPrimTypes{0};

        // Constructors //
    public:
        // `Block` module
        static inline Ptr newBlockModule(NodeId nodeId, Ptr parent, DefId nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Block, parent, nodeId, nearestModDef);
        }

        // `Def` module
        static inline Ptr newDefModule(const DefId & defId, Ptr parent, DefId nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Def, parent, defId, nearestModDef);
        }

    public:
        void assertKind(ModuleKind kind) const;
        auto getNodeId() const;
        const DefId & getDefId() const;
        const NSMap & getNS(Namespace ns) const;
        NSMap & getNS(Namespace ns);

        bool has(Namespace nsKind, const Symbol & name) const;
        NameBinding::Opt find(Namespace nsKind, const Symbol & name) const;
        PerNS<NameBinding::Opt> findAll(const Symbol & name) const;

        /// @deprecated
//        DefId::Opt findDefOnly(Namespace nsKind, const Symbol & name) const;
//        PerNS<DefId::Opt> findAllDefOnly(const Symbol & name) const;

        /**
         * @brief Try to define a new definition in module
         * @return Old definition in case if redefined, None otherwise
         */
        NameBinding::Opt tryDefine(Namespace ns, const Symbol & name, const DefId & defId) {
            const auto & defined = getNS(ns).emplace(name, NameBinding {defId});
            // Note: emplace returns `pair<new element iterator, true>` if new element added
            //  and `pair<old element iterator, false>` if tried to re-emplace
            if (not defined.second) {
                // Return old def id
                return defined.first->second;
            }
            return None;
        }

        NameBinding::Opt addFuncOverload(const Symbol & baseName, const FuncOverloadId & funcOverloadId) {
            // When we try to add already defined function overload -- it is okay.
            // But we cannot define function overload if some non-function definition uses its name.
            const auto & defined = getNS(Namespace::Value).emplace(baseName, NameBinding {funcOverloadId});
            if (not defined.second and defined.first->second.isTarget()) {
                // User tried to define function with name taken by non-function item
                return defined.first->second;
            }
            return None;
        }

    public:
        static inline Symbol getInitName(const ast::Init & init) {
            /// TODO: Use real `init` span
            return getFuncSuffix(init.sig);
        }

        static inline Symbol getImplName(const ast::Node & node) {
            return span::Interner::getInstance().intern("%impl_" + std::to_string(node.id.val));
        }

        /**
         * @brief Get suffix of form like `(label1:label2:_:...)`
         */
        static inline Symbol getFuncSuffix(const ast::FuncSig & sig) {
            // TODO!: Optimize some way :)

            std::string name = "(";

            std::vector<Symbol> labels;
            for (const auto & param : sig.params) {
                if (param.label.some()) {
                    labels.emplace_back(param.label.unwrap().unwrap().sym);
                } else if (param.pat.unwrap()->kind == ast::PatKind::Ident) {
                    // If no label present and IdentPat (such as `ref? mut? IDENT @ Pattern`) used -- use IDENT
                    labels.emplace_back(ast::Pattern::as<ast::IdentPat>(param.pat.unwrap())->name.unwrap().sym);
                } else {
                    labels.emplace_back(Symbol::fromKw(span::Kw::Underscore));
                }
            }

            std::sort(labels.begin(), labels.end(), [](const Symbol & lhs, const Symbol & rhs) {
                return lhs < rhs;
            });

            for (const auto & label : labels) {
                name += label.toString() + ":";
            }

            name += ")";
            return Symbol::intern(name);
        }

        /**
         * @brief Get suffix of form like `(label1:label2:_:...)` from function call expression or
         *  disambiguated invocation (such as `foo(label1:label2)` in case when `foo` has multiple overloads).
         * @param args Call arguments
         * @return pair of `gotLabels` status (if got named label in call, such as "name: value") and interned suffix
         */
        static inline std::pair<bool, Symbol> getCallSuffix(const ast::Arg::List & args) {
            // Build suffix (from labels) and visit arguments
            bool gotLabels = false;
            std::string suffix = "(";
            for (const auto & arg : args) {
                if (arg.name.some()) {
                    gotLabels = true;
                    suffix += arg.name.unwrap().unwrap().sym.toString() + ":";
                } else {
                    suffix += "_:";
                }
            }
            suffix += ")";

            return {gotLabels, Symbol::intern(suffix)};
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
                case Namespace::Any: return "[ANY]";
            }
        }
    };
}

#endif // JACY_RESOLVE_MODULE_H
