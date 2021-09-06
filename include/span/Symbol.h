#ifndef JACY_SPAN_SYMBOL_H
#define JACY_SPAN_SYMBOL_H

#include <cstdint>
#include <map>
#include <vector>

namespace jc::span {
    struct SymbolId {
        using ValueT = uint32_t;

        ValueT val;
    };
    
    struct Symbol {
        using SymMap = std::map<std::string, Symbol>;

        SymbolId id;
    };

    class Interner {
    public:
        Interner() = default;

    private:
        Symbol::SymMap symbols;
        std::vector<std::string> internedStrings;
    };
}

#endif // JACY_SPAN_SYMBOL_H
