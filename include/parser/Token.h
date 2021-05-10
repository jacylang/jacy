#ifndef JACY_TOKEN_H
#define JACY_TOKEN_H

#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>

#include "utils/arr.h"
#include "parser/Span.h"
#include "session/Session.h"

/**
 * WWS means Without whitespace
 * It is a token without followed by whitespace
 */

namespace jc::parser {
    struct Token;
    using token_list = std::vector<Token>;

    struct Location {
        uint32_t offset;
        uint32_t line;
        uint64_t col;

        std::string toString() const {
            return std::to_string(line) + ":" + std::to_string(col);
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
        StringLiteral,
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
        AsQM,
        Break,
        Catch,
        Class,
        Const,
        Continue,
        Do,
        Elif,
        Else,
        Enum,
        Export,
        False,
        Finally,
        For,
        Func,
        Get,
        If,
        Import,
        In,
        Init,
        NotIn,
        Infix,
        Is,
        Loop,
        NotIs,
        Null,
        Object,
        Operator,
        Return,
        Prefix,
        Private,
        Protected,
        Public,
        Set,
        Super,
        This,
        Throw,
        To,
        True,
        Try,
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
        bool isModifier() const;
        span_len len() const;

        Span span(const session::Session & sess) const;

        // Debug //
        static std::string typeToString(const Token & token);
        std::string typeToString() const;
        std::string toString(bool withLoc = false) const;
    };
}

#endif // JACY_TOKEN_H
