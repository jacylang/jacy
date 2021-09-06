#ifndef JACY_SPAN_SYMBOL_H
#define JACY_SPAN_SYMBOL_H

#include <cstdint>
#include <map>
#include <vector>
#include <string>

#include "data_types/Option.h"
#include "log/utils.h"

namespace jc::span {
    class Interner;

    // TODO!!!: Intern keywords

    /// Keywords
    /// NOTE: Order matters!!!
    ///  We use discriminants to access specific interned keyword and to map string to keyword
    enum class KW : uint8_t {
        Empty = 0,
        Root,
        Underscore,

        And,
        As,
        Async,
        Await,
        Break,
        Const,
        Continue,
        Do,
        Elif,
        Else,
        Enum,
        False,
        For,
        Func,
        If,
        Impl,
        Import,
        In,
        Infix,
        Init,
        Loop,
        Match,
        Mod,
        Move,
        Mut,
        Not,
        Of,
        Or,
        Return,
        Party,
        Pub,
        Ref,
        Self,
        Static,
        Struct,
        Super,
        This,
        Trait,
        True,
        Type,
        Use,
        Let,
        Where,
        While,
    };

    struct SymbolId {
        using ValueT = uint32_t;

        ValueT val;
    };

    struct Symbol {
        using Opt = Option<Symbol>;
        using SymMap = std::map<std::string, Symbol>;

        SymbolId id;

        std::string toString(const Interner & interner) const;

        static const std::map<KW, std::string> keywords;
    };

    class Interner {
    public:
        Interner();

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
        /// Maps Symbol name to its value
        Symbol::SymMap symbols;

        /// Stores all interned strings, SymbolId points to its index
        std::vector<std::string> internedStrings;
    };
}

#endif // JACY_SPAN_SYMBOL_H
