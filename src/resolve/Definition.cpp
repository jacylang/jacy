#include "resolve/Definition.h"

namespace jc::resolve {
    const DefIndex DefIndex::ROOT_INDEX = 0;
    const DefId DefId::ROOT_DEF_ID = DefId(DefIndex::ROOT_INDEX);

    std::string nsToString(Namespace ns) {
        switch (ns) {
            case Namespace::Value: return "value";
            case Namespace::Type: return "type";
            case Namespace::Lifetime: return "lifetime";
            case Namespace::Any: return "[ANY]";
        }
    }
}
