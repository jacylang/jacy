#ifndef JACY_SRC_TYPECK_CONVERT_CONVERTER_H
#define JACY_SRC_TYPECK_CONVERT_CONVERTER_H

#include "session/Session.h"
#include "utils/arr.h"
#include "hir/nodes/types.h"
#include "typeck/type/types.h"

namespace jc::typeck {
    using resolve::PrimType;

    class Converter {
    public:
        Converter(const sess::Session::Ptr & sess) : sess {sess}, tyCtx {sess->tyCtx} {}

    public:
        Ty convert(const hir::Type & type);
        Ty convertPath(const hir::TypePath & typePath);
        Ty convertTuple(const hir::TupleType & tupleType);
        Ty convertPrimType(PrimType primType);

    private:
        sess::Session::Ptr sess;
        TypeContext & tyCtx;
    };
}

#endif // JACY_SRC_TYPECK_CONVERT_CONVERTER_H
