#ifndef JACY_TOKEN_H
#define JACY_TOKEN_H

#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>

#include "utils/arr.h"
#include "span/Span.h"
#include "span/Symbol.h"
#include "data_types/Option.h"
#include "parser/ParseSess.h"

/**
 * WWS means Without whitespace
 * It is a token without followed by whitespace
 */

namespace jc::parser {
    struct Location {
        uint32_t line{0};
        uint32_t col{0};
    };

    /// Note: Some keywords marked with RNU (Reserved but Not Used)

    enum class TokenKind : uint8_t {
        Eof,
        Lit,
        Id,

        // Operators //
        Assign,                     // =
        AddAssign,                  // +=
        SubAssign,                  // -=
        MulAssign,                  // *=
        DivAssign,                  // /=
        ModAssign,                  // %=
        PowerAssign,                // **=
        ShlAssign,                  // <<=
        ShrAssign,                  // >>=
        BitAndAssign,               // &=
        BitOrAssign,                // |=
        XorAssign,                  // ^=
        Add,                        // +
        Sub,                        // -
        Mul,                        // *
        Div,                        // /
        Rem,                        // %
        Power,                      // **
        Or,                         // or (placed in keywords)
        And,                        // and (placed in keywords)
        Shl,                        // <<
        Shr,                        // >>
        Ampersand,                  // &
        BitOr,                      // |
        Xor,                        // ^
        Inv,                        // ~
        Eq,                         // ==
        NotEq,                      // !=
        LAngle,                     // <
        RAngle,                     // >
        LE,                         // <=
        GE,                         // >=
        Spaceship,                  // <=>
        RefEq,                      // ===
        RefNotEq,                   // !==
        Range,                      // ..
        RangeEQ,                    // ..=
        Dot,                        // .
        Path,                       // ::
        Spread,                     // ...
        Pipe,                       // |>
        Dollar,                     // $
        At,                         // @
        Backslash,                  // \

        // Punctuations //
        Semi,                       // ;
        Arrow,                      // ->
        DoubleArrow,                // =>
        LParen,                     // (
        RParen,                     // )
        LBrace,                     // {
        RBrace,                     // }
        LBracket,                   // [
        RBracket,                   // ]
        Comma,                      // ,
        Colon,                      // :
        Quest,                      // ?
        Backtick,                   // `

        // Keywords //
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
        Of,         // (RNU)
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
        Wildcard,

        None,
    };

    struct TokLit {
        enum class Kind {
            Bool,
            DecLiteral,
            BinLiteral,
            OctLiteral,
            HexLiteral,
            FloatLiteral,
            SQStringLiteral, // Single-quote string literal
            DQStringLiteral, // Double-quote string literal
        } kind;

        // Interned literal string value
        span::Symbol sym;

        // Suffix used by int and float types (such as in `123u32`)
        span::Symbol::Opt suffix;
    };

    struct Token {
        using ValueT = std::variant<std::monostate, span::Symbol, TokLit>;
        using List = std::vector<Token>;
        using Opt = Option<Token>;

        Token() {}
        Token(
            TokenKind kind,
            ValueT && val
        ) : kind{kind},
            val{std::move(val)} {}

        TokenKind kind{TokenKind::None};
        ValueT val {std::monostate {}};
        span::Span span;

        static const std::map<std::string, TokenKind> keywords;
        static const std::map<TokenKind, std::string> tokenKindStrings;
        static const std::vector<TokenKind> assignOperators;

        span::Symbol asSymbol() const {
            return std::get<span::Symbol>(val);
        }

        const TokLit & asLit() const {
            return std::get<TokLit>(val);
        }

        bool is(TokenKind kind) const;
        bool isAssignOp() const;
        bool isLiteral() const;
        bool isKw() const; // Note: Use only for errors, not for general use
        bool isPathIdent() const;

        std::string toString(const span::Interner & interner, bool prettyQuotes = true) const;
        static std::string kindToString(TokenKind kind);
        static std::string kindToString(const Token & token);
        std::string kindToString() const;

        static std::string listKindToString(const Token::List & tokens);

        // Debug //
        std::string dump(const span::Interner & interner, bool withLoc = false) const;
    };
}

#endif // JACY_TOKEN_H
