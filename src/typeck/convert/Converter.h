#ifndef JACY_SRC_TYPECK_CONVERT_CONVERTER_H
#define JACY_SRC_TYPECK_CONVERT_CONVERTER_H

#include "typeck/type/types.h"
#include "hir/nodes/types.h"
#include "session/Session.h"

namespace jc::typeck {
    class Converter {
    public:
        Converter(const sess::Session::Ptr & sess) : sess {sess} {}

    public:
        Ty convert(const hir::Type & type);

    private:
        sess::Session::Ptr sess;
    };
}

#endif // JACY_SRC_TYPECK_CONVERT_CONVERTER_H
