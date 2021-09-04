#include "hir/nodes/Type.h"

namespace jc::hir {
    const auto Type::INFER = std::make_unique<Type>(TypeKind::Infer, HirId::DUMMY, Span {});
}
