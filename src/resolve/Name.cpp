#include "resolve/Name.h"

namespace jc::resolve {
    const std::map<Name::Kind, const std::string> Name::kindsStrings = {
        {Name::Kind::Const, "Const"},
        {Name::Kind::Struct, "Struct"},
        {Name::Kind::Trait, "Trait"},
        {Name::Kind::Local, "Local"},
        {Name::Kind::TypeParam, "TypeParam"},
        {Name::Kind::Lifetime, "Lifetime"},
        {Name::Kind::ConstParam, "ConstParam"},
        {Name::Kind::Func, "Func"},
        {Name::Kind::Enum, "Enum"},
        {Name::Kind::TypeAlias, "TypeAlias"},
        {Name::Kind::Param, "Param"},
    };

    decl_result Rib::declare(const std::string & name, Name::Kind kind, ast::node_id nodeId) {
        auto & ns = getNSForName(kind);
        const auto & found = ns.find(name);
        if (found == ns.end()) {
            ns[name] = std::make_shared<Name>(kind, nodeId);
            return dt::None;
        }
        return found->second;
    }

    decl_result Rib::resolve(const std::string & name, RibNamespace nsKind) {
        auto & ns = getNS(nsKind);
        const auto & found = ns.find(name);
        if (found == ns.end()) {
            return dt::None;
        }
        return found->second;
    }

    ns_map & Rib::getNSForName(Name::Kind kind) {
        switch (kind) {
            case Name::Kind::Const:
            case Name::Kind::Param:
            case Name::Kind::Local:
            case Name::Kind::ConstParam:
            case Name::Kind::Func: {
                return valueNS;
            }
            case Name::Kind::Enum:
            case Name::Kind::Struct:
            case Name::Kind::Trait:
            case Name::Kind::TypeAlias:
            case Name::Kind::TypeParam: {
                return typeNS;
            }
            case Name::Kind::Lifetime: {
                return lifetimeNS;
            }
        }
    }

    ns_map & Rib::getNS(RibNamespace nsKind) {
        switch (nsKind) {
            case RibNamespace::Value: return valueNS;
            case RibNamespace::Type: return typeNS;
            case RibNamespace::Lifetime: return lifetimeNS;
        }
    }

    void Rib::bindMod(module_ptr module) {
        boundModule = std::move(module);
    }
}
