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
        Error,

        Eof,
        Whitespace,
        Tab,
        NL,
        LineComment,
        BlockComment,

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
        Not,                        // not
        Or,                         // or
        And,                        // and
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

        None,
    };

    enum class PairedTokens {
        Paren,
        Brace,
        Bracket,
        Angle,
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

        friend std::ostream & operator<<(std::ostream & os, const TokLit & lit) {
            // TODO: String literal suffixes?

            switch (lit.kind) {
                case Kind::Bool: {
                    return os << lit.sym.toString();
                }
                case Kind::DecLiteral:
                case Kind::BinLiteral:
                case Kind::OctLiteral:
                case Kind::HexLiteral:
                case Kind::FloatLiteral: {
                    return os << lit.sym.toString() << (lit.suffix.some() ? lit.suffix.unwrap().toString() : "");
                }
                case Kind::SQStringLiteral: {
                    return os << "'" << lit.sym.toString() << "'";
                }
                case Kind::DQStringLiteral: {
                    return os << "'" << lit.sym.toString() << "'";
                }
            }
        }
    };

    struct Token {
        /**
         * ValueT variants:
         * - `None` for tag-only tokens like operators
         * - `Symbol` for identifiers and keywords
         * - `TokLit` for literals
         */
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

        static const std::map<TokenKind, std::string> tokenKindStrings;
        static const std::vector<TokenKind> assignOperators;

        span::Symbol asSymbol() const {
            return std::get<span::Symbol>(val);
        }

        const TokLit & asLit() const {
            return std::get<TokLit>(val);
        }

        bool is(TokenKind kind) const;
        bool isIdentLike(TokenKind kind, span::Symbol::Opt sym) const;
        bool isKw(span::Kw kw) const;
        bool isHidden() const;

        bool isSomeOp() const;
        bool isAssignOp() const;
        bool isLiteral() const;
        bool isSomeKeyword() const; // Note: Use only for errors, not for general use
        bool isPathIdent() const;

        static Option<TokenKind> keywordOperator(span::Kw kw);
        static std::tuple<TokenKind, TokenKind> getTokenPairs(PairedTokens pair);

        std::string repr(bool prettyQuotes = true) const;
        static std::string kindToString(TokenKind kind);
        static std::string kindToString(const Token & token);
        std::string kindToString() const;

        static std::string listKindToString(const Token::List & tokens);

        // Pretty print token (Don't print debug-like representation)
        friend std::ostream & operator<<(std::ostream & os, const Token & token) {
            switch (token.kind) {
                case TokenKind::Eof: {
                    return os;
                }
                case TokenKind::NL: {
                    return os << "\n";
                }
                case TokenKind::Whitespace: {
                    return os << " ";
                }
                case TokenKind::Tab: {
                    return os << "\t";
                }
                case TokenKind::LineComment: {
                    return os << "//" << token.asSymbol().toString();
                }
                case TokenKind::BlockComment: {
                    return os << "/*" << token.asSymbol().toString() << "*/";
                }
                case TokenKind::Id: {
                    return os << token.asSymbol().toString();
                }
                case TokenKind::Lit: {
                    return os << token.asLit();
                }
                default: {
                    return os << tokenKindStrings.at(token.kind);
                }
            }
        }

        /**
         * @brief Partial equality, i.e. the equality is not absolute.
         *  Error always != any other Error
         *  Hidden tokens (comments, new-lines, white-spaces, etc.) are equal
         *
         * @param other Other token to compare this one with
         * @return Comparison result
         */
        bool operator==(const Token & other) const {
            if (kind != other.kind) {
                return false;
            }

            switch (kind) {
                case TokenKind::Error: {
                    return false;
                }
                case TokenKind::Eof: {
                    return true;
                }
                case TokenKind::Whitespace:
                case TokenKind::Tab:
                case TokenKind::NL:
                case TokenKind::LineComment:
                case TokenKind::BlockComment: {
                    return true;
                }
                case TokenKind::Lit: {
                    return asLit().kind == other.asLit().kind
                        and asLit().sym == other.asLit().sym
                        and asLit().suffix == other.asLit().suffix;
                }
                case TokenKind::Id: {
                    return asSymbol() == other.asSymbol();
                }
                default: {
                    return true;
                }
            }
        }

        // Debug //
        std::string dump(bool withLoc = true) const;
    };
}

#endif // JACY_TOKEN_H
