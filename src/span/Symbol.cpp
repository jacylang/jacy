#include "span/Symbol.h"

namespace jc::span {
    std::string Symbol::toString(const Interner & interner) const {
        return interner.getString(*this);
    }
}
