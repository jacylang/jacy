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
        using Opt = Option<Symbol>;
        using SymMap = std::map<std::string, Symbol>;

        SymbolId id;
    };

    class Interner {
    public:
        Interner() = default;

        Symbol intern(const std::string & str) {
            const auto & found = symbols.find(str);
            if (found != symbols.end()) {
                return found->second;
            }

            auto sym = Symbol {static_cast<SymbolId::ValueT>(symbols.size())};

            symbols.emplace(str, sym);
            internedStrings.emplace_back(str);

            return sym;
        }

        const std::string & getString(const Symbol & sym) const {
            return internedStrings.at(sym.id.val);
        }

    private:
        Symbol::SymMap symbols;
        std::vector<std::string> internedStrings;
    };
}

#endif // JACY_SPAN_SYMBOL_H
