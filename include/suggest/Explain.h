#ifndef JACY_EXPLAIN_H
#define JACY_EXPLAIN_H

#include <cstdint>

namespace jc::sugg {
    struct EID {
        using ValueT = uint32_t;

        EID(ValueT val) : val{val} {}

        static const EID NoneEID;

        ValueT val;
    };

}

#endif // JACY_EXPLAIN_H
