#ifndef JACY_TOKEN_H
#define JACY_TOKEN_H

#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>

#include "utils/arr.h"
#include "span/Span.h"
#include "data_types/Option.h"
#include "parser/ParseSess.h"

/**
 * WWS means Without whitespace
 * It is a token without followed by whitespace
 */

namespace jc::parser {
    struct Token;
    using token_list = std::vector<Token>;
    using opt_token = dt::Option<Token>;

    struct Location {
        uint32_t line{0};
        uint32_t col{0};
    };

    enum class TokenKind : uint8_t {
        Eof,
        DecLiteral,
        BinLiteral,
        OctLiteral,
        HexLiteral,
        FloatLiteral,
        SQStringLiteral, // Single-quote string literal
        DQStringLiteral, // Double-quote string literal
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
        NullishAssign,              // ??=
        Add,                        // +
        Sub,                        // -
        Mul,                        // *
        Div,                        // /
        Mod,                        // %
        Power,                      // **
        Or,                         // or
        And,                        // and
        NullCoalesce,               // ??
        Shl,                        // <<
        Shr,                        // >>
        BitAnd,                     // &
        BitOr,                      // |
        Xor,                        // ^
        Inv,                        // ~
        Not,                        // !
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
        At_WWS,                     // @ Without whitespace

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
        In,
        Init,
        NotIn,
        Infix,
        Loop,
        Match,
        Module,
        Move,
        Mut,
        Return,
        Party,
        Pri,
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
        Union,
        Unsafe,
        Use,
        Let,
        Where,
        While,
        Wildcard,
        Yield,

        None,
    };

    struct Token {
        Token() {}
        Token(
            TokenKind kind,
            std::string val
        ) : kind(kind),
            val(std::move(val)) {}

        TokenKind kind{TokenKind::None};
        std::string val{""};
        span::Span span;

        static const std::map<std::string, TokenKind> keywords;
        static const std::map<TokenKind, std::string> tokenKindStrings;
        static const std::vector<TokenKind> assignOperators;
        static const std::vector<TokenKind> literals;

        bool is(TokenKind kind) const;
        bool isAssignOp() const;
        bool isLiteral() const;
        bool isKw() const; // Note: Use only for errors, not for general use

        std::string toString(bool prettyQuotes = true) const;
        static std::string kindToString(TokenKind kind);
        static std::string kindToString(const Token & token);
        std::string kindToString() const;

        static std::string listKindToString(const token_list & tokens);

        // Debug //
        std::string dump(bool withLoc = false) const;
    };
}

#endif // JACY_TOKEN_H
