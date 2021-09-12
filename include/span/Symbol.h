#ifndef JACY_SPAN_SYMBOL_H
#define JACY_SPAN_SYMBOL_H

#include <cstdint>
#include <map>
#include <vector>
#include <string>

#include "data_types/Option.h"
#include "log/utils.h"
#include "utils/arr.h"

namespace jc::span {
    class Interner;

    // TODO!!!: Intern keywords

    /// Keywords
    /// NOTE: Order matters!!!
    ///  We use discriminants to access specific interned keyword and to map string to keyword
    enum class Kw : uint8_t {
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

        bool operator==(const SymbolId & other) const {
            return val == other.val;
        }

        bool operator<(const SymbolId & other) const {
            return val < other.val;
        }
    };

    struct Symbol {
        using Opt = Option<Symbol>;
        using SymMap = std::map<std::string, Symbol>;

        SymbolId id;

        std::string toString() const;

        bool operator==(const Symbol & other) const {
            return id == other.id;
        }

        bool operator<(const Symbol & other) const {
            return id < other.id;
        }

        bool operator==(Kw kw) const {
            return std::underlying_type_t<Kw>(kw) == id.val;
        }

        Symbol operator+(const Symbol & other) const;
        Symbol & operator+=(const Symbol & other);

        static Symbol intern(const std::string & str);

        static auto kwAsInt(Kw kw) {
            return static_cast<std::underlying_type_t<Kw>>(kw);
        }

        static Symbol fromKw(Kw kw) {
            return Symbol {kwAsInt(kw)};
        }

        static Symbol empty() {
            return Symbol {kwAsInt(Kw::Empty)};
        }

        bool isKw() const {
            return kwAsInt(Kw::And) <= id.val and kwAsInt(Kw::While) >= id.val;
        }

        bool isPathSeg() const {
            // Check if segment is a path segment keyword and not any another keyword, i.e. user-defined identifier
            return *this == Kw::Super
                or *this == Kw::Party
                or *this == Kw::Self
                or not isKw();
        }

        static std::string kwToString(Kw kw);

        friend std::ostream & operator<<(std::ostream & os, const Symbol & sym);

        static const std::map<Kw, std::string> keywords;
    };

    class Interner {
    public:
        Interner();

        // Singleton //
        Interner(Interner const&) = delete;
        void operator=(Interner const&) = delete;

        static auto & getInstance() {
            static Interner instance {};
            return instance;
        }

    public:
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

        const std::string & get(const Symbol & sym) const {
            return utils::arr::expectAt(internedStrings, sym.id.val, "`Interner::get`");
        }

    private:
        /// Maps Symbol name to its value
        Symbol::SymMap symbols;

        /// Stores all interned strings, SymbolId points to its index
        std::vector<std::string> internedStrings;
    };
}

#endif // JACY_SPAN_SYMBOL_H
