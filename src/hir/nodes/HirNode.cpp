#include "hir/nodes/HirNode.h"

namespace jc::hir {
    const HirId HirId::DUMMY = HirId(resolve::DefId(resolve::DefIndex {0}));
}
