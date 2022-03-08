#ifndef JACY_SRC_TYPECK_CONVERT_CONVERTER_H
#define JACY_SRC_TYPECK_CONVERT_CONVERTER_H

#include "session/Session.h"
#include "utils/arr.h"
#include "hir/nodes/types.h"
#include "typeck/type/types.h"

namespace jc::typeck {
    class Converter {
    public:
        Converter(const sess::Session::Ptr & sess) : sess {sess} {}

    public:
        Ty convert(const hir::Type & type);
        Ty convertPath(const hir::TypePath & typePath);
        Ty convertTuple(const hir::TupleType & tupleType);

    private:
        sess::Session::Ptr sess;
    };
}

#endif // JACY_SRC_TYPECK_CONVERT_CONVERTER_H
