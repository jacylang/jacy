#ifndef JACY_SRC_TYPECK_CONVERT_CONVERTER_H
#define JACY_SRC_TYPECK_CONVERT_CONVERTER_H

#include "utils/arr.h"
#include "hir/nodes/types.h"
#include "typeck/type/types.h"

namespace jc::typeck {
    using resolve::PrimType;

    class TypeContext;

    class Converter {
    public:
        Converter(TypeContext & tyCtx) : tyCtx {tyCtx} {}

    public:
        Ty convert(const hir::Type & type);
        Ty convertPath(const hir::TypePath & typePath);
        Ty convertTuple(const hir::TupleType & tupleType);
        Ty convertPrimType(PrimType primType);

        Type::List convertTypeList(const hir::Type::List & list);

    private:
        TypeContext & tyCtx;
    };
}

#endif // JACY_SRC_TYPECK_CONVERT_CONVERTER_H
