#include "hir/nodes/HirNode.h"

namespace jc::hir {
    const HirId HirId::NONE_HIR_ID = HirId(resolve::DefId(resolve::DefIndex {0}));
}
