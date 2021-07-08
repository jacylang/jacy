#ifndef JACY_RESOLVE_DEFINITION_H
#define JACY_RESOLVE_DEFINITION_H

#include "span/Span.h"
#include "ast/Node.h"

namespace jc::resolve {
    struct Module;
    using module_ptr = std::shared_ptr<Module>;
    using opt_module_ptr = dt::Option<module_ptr>;
    using def_id = size_t;
    using opt_def_id = dt::Option<def_id>;

    enum class Namespace {
        Value,
        Type,
        Lifetime,
    };

    enum class DefKind {
        Dir,
        File,
        Root,

        Const,
        ConstParam,
        Enum,
        Func,
        Impl,
        Lifetime,
        Mod,
        Struct,
        Trait,
        TypeAlias,
        TypeParam,
        Variant,
    };

    enum class NameUsage {
        Type,
        Expr,
        Lifetime,
    };

    // Stolen from Rust's source code
    // As, actually, mostly everything else ❤🔥
    template<class T>
    struct PerNS {
        T value;
        T type;
        T lifetime;
    };

    struct Def {
        Def(
            DefKind kind,
            const dt::Option<span::Span> & nameSpan,
            ast::opt_node_id nameNodeId
        ) : kind(kind),
            nameNodeId(nameNodeId),
            nameSpan(nameSpan) {}

        DefKind kind;
        const ast::opt_node_id nameNodeId;
        const dt::Option<span::Span> nameSpan;

        static inline Namespace getNS(DefKind kind) {
            switch (kind) {
                case DefKind::Enum:
                case DefKind::Mod:
                case DefKind::Trait:
                case DefKind::TypeAlias:
                case DefKind::TypeParam:
                case DefKind::Dir:
                case DefKind::File:
                case DefKind::Struct:
                case DefKind::Variant: {
                    return Namespace::Type;
                }
                case DefKind::Const: {
                case DefKind::ConstParam:
                case DefKind::Func:
                    return Namespace::Value;
                }
                case DefKind::Lifetime: {
                    return Namespace::Lifetime;
                }
                default:;
            }

            common::Logger::notImplemented("Definition::getNS");
        }

        static std::string kindStr(DefKind kind) {
            switch (kind) {
                case DefKind::Const:
                    return "`const`";
                case DefKind::Struct:
                    return "`struct`";
                case DefKind::Trait:
                    return "`trait`";
                case DefKind::TypeParam:
                    return "type parameter";
                case DefKind::Lifetime:
                    return "lifetime parameter";
                case DefKind::ConstParam:
                    return "`const` parameter";
                case DefKind::Func:
                    return "`func`";
                case DefKind::Enum:
                    return "`enum`";
                case DefKind::TypeAlias:
                    return "`type` alias";
                case DefKind::Dir:
                    return "[DIR]";
                case DefKind::File:
                    return "[FILE]";
                case DefKind::Root:
                    return "[ROOT]";
                case DefKind::Impl:
                    return "`impl`";
                case DefKind::Mod:
                    return "`mod`";
                case DefKind::Variant:
                    return "`enum` variant";
                default: return "[NO REPRESENTATION (bug)]";
            }
        }

        std::string kindStr() const {
            return kindStr(kind);
        }

        static bool isUsableAs(DefKind kind, NameUsage usage) {
            if (usage == NameUsage::Lifetime) {
                switch (kind) {
                    case DefKind::Lifetime:
                        return true;
                    default:
                        return false;
                }
            }
            if (usage == NameUsage::Type) {
                switch (kind) {
                    case DefKind::Struct:
                    case DefKind::Trait:
                    case DefKind::TypeAlias:
                    case DefKind::TypeParam: {
                        return true;
                    }
                    default: return false;
                }
            }
            if (usage == NameUsage::Expr) {
                switch (kind) {
                    case DefKind::Const:
                    case DefKind::ConstParam:
                    case DefKind::Func: {
                        return true;
                    }
                    default: return false;
                }
            }

            common::Logger::notImplemented("Definition::isUsableAs");
        }

        bool isUsableAs(NameUsage usage) const {
            return isUsableAs(kind, usage);
        }

        static std::string usageToString(NameUsage usage) {
            switch (usage) {
                case NameUsage::Type: return "type";
                case NameUsage::Expr: return "expression";
                case NameUsage::Lifetime: return "lifetime";
                default: return "[NO REPRESENTATION (bug)]";
            }
        }

        // Debug //
        friend std::ostream & operator<<(std::ostream & os, const Def & def) {
            os << def.kindStr();
            if (def.nameNodeId) {
                os << " [" << def.nameNodeId.unwrap() << "]";
            }
            return os;
        }
    };

    struct DefStorage {
        const Def & getDef(def_id defId) const {
            try {
                return defs.at(defId);
            } catch (std::out_of_range & e) {
                common::Logger::devPanic("Called `DefStorage::getDef` with non-existent `defId`");
            }
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

#endif // JACY_RESOLVE_DEFINITION_H
