#include "parser/Token.h"

namespace jc::parser {
    const std::map<TokenKind, std::string> Token::tokenKindStrings = {
        {TokenKind::Eof,          "EOF"},
        {TokenKind::Id,           "ID"},

        // Operators //
        {TokenKind::Assign,       "="},
        {TokenKind::AddAssign,    "+="},
        {TokenKind::SubAssign,    "-="},
        {TokenKind::MulAssign,    "*="},
        {TokenKind::DivAssign,    "/="},
        {TokenKind::ModAssign,    "%="},
        {TokenKind::PowerAssign,  "**="},
        {TokenKind::ShlAssign,    "<<="},
        {TokenKind::ShrAssign,    ">>="},
        {TokenKind::BitAndAssign, "&="},
        {TokenKind::BitOrAssign,  "|="},
        {TokenKind::XorAssign,    "^="},
        {TokenKind::Add,          "+"},
        {TokenKind::Sub,          "-"},
        {TokenKind::Mul,          "*"},
        {TokenKind::Div,          "/"},
        {TokenKind::Rem,          "%"},
        {TokenKind::Power,        "**"},
        {TokenKind::Or,           "or"},
        {TokenKind::And,          "and"},
        {TokenKind::Shl,          "<<"},
        {TokenKind::Shr,          ">>"},
        {TokenKind::Ampersand,    "&"},
        {TokenKind::BitOr,        "|"},
        {TokenKind::Xor,          "^"},
        {TokenKind::Inv,          "~"},
        {TokenKind::Eq,           "=="},
        {TokenKind::NotEq,        "!="},
        {TokenKind::LAngle,       "<"},
        {TokenKind::RAngle,       ">"},
        {TokenKind::LE,           "<="},
        {TokenKind::GE,           ">="},
        {TokenKind::Spaceship,    "<=>"},
        {TokenKind::RefEq,        "==="},
        {TokenKind::RefNotEq,     "!=="},
        {TokenKind::Range,        ".."},
        {TokenKind::RangeEQ,      "..="},
        {TokenKind::Dot,          "."},
        {TokenKind::Path,         "::"},
        {TokenKind::Spread,       "..."},
        {TokenKind::Pipe,         "|>"},
        {TokenKind::Dollar,       "$"},
        {TokenKind::At,           "@"},

        // Punctuations //
        {TokenKind::Semi,         ";"},
        {TokenKind::Arrow,        "->"},
        {TokenKind::DoubleArrow,  "=>"},
        {TokenKind::LParen,       "("},
        {TokenKind::RParen,       ")"},
        {TokenKind::LBrace,       "{"},
        {TokenKind::RBrace,       "}"},
        {TokenKind::LBracket,     "["},
        {TokenKind::RBracket,     "]"},
        {TokenKind::Comma,        ","},
        {TokenKind::Colon,        ":"},
        {TokenKind::Quest,        "?"},
        {TokenKind::Backtick,     "`"},
        {TokenKind::Backslash,    "\\"},
    };

    const std::vector<TokenKind> Token::assignOperators = {
        TokenKind::Assign,
        TokenKind::AddAssign,
        TokenKind::SubAssign,
        TokenKind::MulAssign,
        TokenKind::DivAssign,
        TokenKind::ModAssign,
        TokenKind::PowerAssign,
        TokenKind::ShlAssign,
        TokenKind::ShrAssign,
        TokenKind::BitAndAssign,
        TokenKind::BitOrAssign,
        TokenKind::XorAssign,
    };

    bool Token::is(TokenKind kind) const {
        return this->kind == kind;
    }

    bool Token::isKw(span::KW kw) const {
        return asSymbol() == kw;
    }

    bool Token::isAssignOp() const {
        return utils::arr::has(assignOperators, kind);
    }

    bool Token::isLiteral() const {
        return kind == TokenKind::Lit;
    }

    bool Token::isSomeKeyword() const {
        return kind == TokenKind::Id and asSymbol().isKw();
    }

    bool Token::isPathIdent() const {
        return kind == TokenKind::Id or asSymbol().isPathSeg();
    }

    std::string Token::kindToString(TokenKind kind) {
        const auto found = tokenKindStrings.find(kind);
        if (found != tokenKindStrings.end()) {
            return found->second;
        }

        for (const auto & kw : keywords) {
            if (kw.second == kind) {
                return kw.first;
            }
        }

        return "No representation";
    }

    std::string Token::kindToString(const Token & token) {
        return kindToString(token.kind);
    }

    std::string Token::kindToString() const {
        return kindToString(*this);
    }

    std::string Token::listKindToString(const Token::List & tokens) {
        std::string str;
        for (const auto & token : tokens) {
            str += token.kindToString();
        }
        return str;
    }

    // FIXME: Update `toString` and `dump` when `Session` will be global state and thus `Interner` will be globally accessible

    std::string Token::toString(bool prettyQuotes) const {
        std::string str;
        if (prettyQuotes) {
            str += "`";
        }
        switch (kind) {
            case TokenKind::Lit:
            case TokenKind::Id: {
                str += asSymbol().toString();
            } break;
            default: {
                str += kindToString();
            }
        }
        return str + "`";
    }

    std::string Token::dump(bool withSpan) const {
        std::string str = kindToString();

        switch (kind) {
            case TokenKind::Lit:
            case TokenKind::Id: {
                str += ":'" + asSymbol().toString() + "'";
            }
            default:;
        }

        if (withSpan) {
            str += " at " + span.toString();
        }

        return str;
    }
}
