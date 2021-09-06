#ifndef JACY_SPAN_SYMBOL_H
#define JACY_SPAN_SYMBOL_H

#include <cstdint>

namespace jc::span {
    struct SymbolId {
        using ValueT = uint32_t;

        ValueT val;
    };
    
    struct Symbol {
        SymbolId id;
    };
}

#endif // JACY_SPAN_SYMBOL_H
