#include "resolve/Name.h"

namespace jc::resolve {
    decl_result Rib::declare(const std::string & name, Name::Kind kind, ast::node_id nodeId) {
        auto & ns = getNSForName(kind);
        const auto & found = ns.find(name);
        if (found == ns.end()) {
            ns.emplace(name, std::make_shared<Name>(kind, nodeId));
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
}
