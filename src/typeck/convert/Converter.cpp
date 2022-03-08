#include "Converter.h"

namespace jc::typeck {
    Ty Converter::convert(const hir::Type & type) {
        switch (type.kind->kind) {
            case hir::TypeKind::Kind::Infer: {
                return sess->typeCtx.makeInfer();
            }
            case hir::TypeKind::Kind::Tuple: {
                break;
            }
            case hir::TypeKind::Kind::Func:
                break;
            case hir::TypeKind::Kind::Slice:
                break;
            case hir::TypeKind::Kind::Array: {
                break;
            }
            case hir::TypeKind::Kind::Path:
                break;
            case hir::TypeKind::Kind::Unit:
                break;
        }
    }
}
