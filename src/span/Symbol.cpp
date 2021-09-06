#include "span/Symbol.h"
#include "session/Session.h"

namespace jc::span {
    std::string Symbol::toString(const Interner & interner) const {
        return interner.getString(*this);
    }
}
