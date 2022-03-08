#ifndef JACY_SRC_TYPECK_CONVERT_CONVERTER_H
#define JACY_SRC_TYPECK_CONVERT_CONVERTER_H

#include "typeck/type/types.h"
#include "hir/nodes/types.h"

namespace jc::typeck {
    class Converter {
    public:
        Converter() = default;

    public:
        Type convert(const hir::Type & type);

        TypeKind::Ptr convertTypeKind(const hir::TypeKind::Ptr & type);
    };
}

#endif // JACY_SRC_TYPECK_CONVERT_CONVERTER_H
