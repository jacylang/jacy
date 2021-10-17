#include "ast/Node.h"

namespace jc::ast {
    const auto NodeId::DUMMY = NodeId {UINT32_MAX};
    const auto NodeId::ROOT_NODE_ID = NodeId {0};
}
