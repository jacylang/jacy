#include "hir/nodes/HirId.h"

namespace jc::hir {
    const HirId HirId::DUMMY = HirId(resolve::DefId(resolve::DefIndex {0}), ChildId::ownerChild());
    const HirId HirId::ROOT = HirId(resolve::DefId::ROOT_DEF_ID, ChildId::ownerChild());
}
