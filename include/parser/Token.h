#ifndef JACY_TOKEN_H
#define JACY_TOKEN_H

#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>

#include "utils/arr.h"
#include "span/Span.h"
#include "session/Session.h"
#include "data_types/Option.h"

/**
 * WWS means Without whitespace
 * It is a token without followed by whitespace
 */

namespace jc::parser {
    struct Token;
    using token_list = std::vector<Token>;
    using opt_token = dt::Option<Token>;

    struct Location {
        uint32_t line;
        uint32_t col;
        uint16_t len;

        span::Span span(sess::sess_ptr sess) const {
            return {line, col, len, sess->fileId};
        }

        std::string toString() const {
            return std::to_string(line + 1) + ":" + std::to_string(col + 1) + " L=" + std::to_string(len);
        }
    };

    enum class TokenKind : uint8_t {
        Eof,
        Nl,
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
        Or,                         // ||
        And,                        // &&
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
        Break,
        Const,
        Continue,
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
        Move,
        Mut,
        Return,
        Pri,
        Pub,
        Static,
        Struct,
        This,
        Trait,
        True,
        Type,
        Val,
        Var,
        When,
        While,

        None,
    };

    struct Token {
        Token() = default;
        Token(
            TokenKind kind,
            std::string val,
            const Location & loc
        ) : kind(kind),
            val(std::move(val)),
            loc(loc) {}

        TokenKind kind{TokenKind::None};
        std::string val{""};

        Location loc{};

        static const std::map<std::string, TokenKind> keywords;
        static const std::map<TokenKind, std::string> tokenKindStrings;
        static const std::vector<TokenKind> assignOperators;
        static const std::vector<TokenKind> literals;

        bool is(TokenKind kind) const;
        bool isAssignOp() const;
        bool isLiteral() const;

        span::Span span(sess::sess_ptr sess) const;

        std::string toString(bool prettyQuotes = true) const;
        static std::string kindToString(TokenKind kind);
        static std::string kindToString(const Token & token);
        std::string kindToString() const;

        // Debug //
        std::string dump(bool withLoc = false) const;
    };
}

#endif // JACY_TOKEN_H
