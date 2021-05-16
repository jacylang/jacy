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

/**
 * WWS means Without whitespace
 * It is a token without followed by whitespace
 */

namespace jc::parser {
    struct Token;
    using token_list = std::vector<Token>;

    struct Location {
        uint32_t line;
        uint32_t col;
        uint16_t len;

        span::Span span(sess::sess_ptr sess) const {
            return {line, col, len, sess->fileId};
        }

        std::string toString() const {
            return std::to_string(line + 1) + ":" + std::to_string(col + 1);
        }
    };

    enum class TokenType {
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
        Inc,                        // ++
        Dec,                        // --
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
        RangeLE,                    // >..
        RangeRE,                    // ..<
        RangeBothE,                 // >.<
        Dot,                        // .
        Path,                       // ::
        SafeCall,                   // ?.
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

        // Keywords //
        As,
        Break,
        Const,
        Continue,
        Elif, // TODO
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
        Mut,
        Return,
        Pri,
        Pub,
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
        Token() {}
        Token(TokenType type, std::string val, const Location & loc) : type(type), val(std::move(val)), loc(loc) {}

        TokenType type{TokenType::None};
        std::string val{""};

        Location loc{};

        static const std::map<std::string, TokenType> keywords;
        static const std::map<TokenType, std::string> tokenTypeStrings;
        static const std::vector<TokenType> assignOperators;
        static const std::vector<TokenType> literals;

        bool is(TokenType type) const;
        bool isAssignOp() const;
        bool isLiteral() const;
//        bool isModifier() const;

        span::Span span(sess::sess_ptr sess) const;

        // Debug //
        static std::string typeToString(TokenType type);
        static std::string typeToString(const Token & token);
        std::string typeToString() const;
        std::string toString(bool withLoc = false) const;
    };
}

#endif // JACY_TOKEN_H
