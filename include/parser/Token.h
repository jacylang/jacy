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
    struct Location {
        uint32_t line{0};
        uint32_t col{0};
    };

    /// Note: Some keywords marked with RNU (Reserved but Not Used)

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
        OP,                         // Custom operator
        And,                        // `and` (placed in keywords)
        Or,                         // `or` (placed in keywords)
        // These operators are custom, anyway we use them in some places in parser,
        //  and comparing enum variants is better than strings.
        // NAME                     // OP (USAGE)

        Assign,                     // = (`func`, `type`, etc.)
        Plus,                       // + (multiple `trait` `impl`'s
        Mul,                        // * (`use` item)
        Ampersand,                  // & (borrowing)
        BitOr,                      // | (closures, `match`)
        LAngle,                     // < (generics)
        RAngle,                     // > (generics)

        // Punctuations //
        Backslash,                  // `\`
        Spread,                     // ...
        Dollar,                     // $
        At,                         // @
        Path,                       // ::
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
        Module,
        Move,
        Mut,
        Of,
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

    struct Token {
        using List = std::vector<Token>;
        using Opt = Option<Token>;

        Token() {}
        Token(
            TokenKind kind,
            std::string val
        ) : kind{kind},
            val{std::move(val)} {}

        TokenKind kind{TokenKind::None};
        std::string val{""};
        span::Span span;

        static const std::map<std::string, TokenKind> keywords;
        static const std::map<TokenKind, std::string> tokenKindStrings;
        static const std::vector<TokenKind> literals;

        bool is(TokenKind kind) const;
        bool isAssignOp() const;
        bool isLiteral() const;
        bool isKw() const; // Note: Use only for errors, not for general use
        bool isPathIdent() const;

        std::string toString(bool prettyQuotes = true) const;
        static std::string kindToString(TokenKind kind);
        static std::string kindToString(const Token & token);
        std::string kindToString() const;

        static std::string listKindToString(const Token::List & tokens);

        // Debug //
        std::string dump(bool withLoc = false) const;
    };
}

#endif // JACY_TOKEN_H
