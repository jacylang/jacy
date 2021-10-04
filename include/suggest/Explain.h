#ifndef JACY_EXPLAIN_H
#define JACY_EXPLAIN_H

#include <cstdint>

namespace jc::sugg {
    struct EID {
        using ValueT = uint32_t;

        EID(ValueT val) : val{val} {}

        static const EID NoneEID;

        ValueT val;

        friend std::ostream & operator<<(std::ostream & os, EID eid) {
            return os << "EID(" << eid.val << ")";
        }

        auto operator==(const EID & eid) const {
            return eid.val == this->val;
        }

        auto operator!=(const EID & eid) const {
            return eid.val != this->val;
        }
    };
}

#endif // JACY_EXPLAIN_H
