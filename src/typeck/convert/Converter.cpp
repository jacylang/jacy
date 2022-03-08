#include "Converter.h"

namespace jc::typeck {
    Type Converter::convert(const hir::Type & type) {
        return Type {convertTypeKind(type.kind)};
    }

    TypeKind::Ptr Converter::convertTypeKind(const hir::TypeKind::Ptr & type) {
        switch (type->kind) {
            case hir::TypeKind::Kind::Infer: {
                return TypeKind::make<Infer>();
            }
            case hir::TypeKind::Kind::Tuple: {
                break;
            }
            case hir::TypeKind::Kind::Func:
                break;
            case hir::TypeKind::Kind::Slice:
                break;
            case hir::TypeKind::Kind::Array:
                break;
            case hir::TypeKind::Kind::Path:
                break;
            case hir::TypeKind::Kind::Unit:
                break;
        }
    }
}
