#include "parser/Token.h"

namespace jc::parser {
    const std::map<TokenKind, std::string> Token::tokenKindStrings = {
        {TokenKind::Eof,          "EOF"},
        {TokenKind::Id,           "ID"},
        {TokenKind::Whitespace,   " "},
        {TokenKind::Tab,          "\t"},
        {TokenKind::NL,           "\n"},

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

    bool Token::isIdentLike(TokenKind kind, span::Symbol::Opt sym) const {
        if (sym.some()) {
            return is(kind) and asSymbol() == sym.unwrap();
        }
        return is(kind);
    }

    bool Token::isKw(span::Kw kw) const {
        return is(TokenKind::Id) and asSymbol().isKw(kw);
    }

    bool Token::isHidden() const {
        return is(TokenKind::Whitespace)
            or is(TokenKind::Tab)
            or is(TokenKind::NL)
            or is(TokenKind::LineComment)
            or is(TokenKind::BlockComment);
    }

    bool Token::isSomeOp() const {
        switch (kind) {
            case TokenKind::Assign:
            case TokenKind::AddAssign:
            case TokenKind::SubAssign:
            case TokenKind::MulAssign:
            case TokenKind::DivAssign:
            case TokenKind::ModAssign:
            case TokenKind::PowerAssign:
            case TokenKind::ShlAssign:
            case TokenKind::ShrAssign:
            case TokenKind::BitAndAssign:
            case TokenKind::BitOrAssign:
            case TokenKind::XorAssign:
            case TokenKind::Add:
            case TokenKind::Sub:
            case TokenKind::Mul:
            case TokenKind::Div:
            case TokenKind::Rem:
            case TokenKind::Power:
            case TokenKind::Or:
            case TokenKind::And:
            case TokenKind::Shl:
            case TokenKind::Shr:
            case TokenKind::Ampersand:
            case TokenKind::BitOr:
            case TokenKind::Xor:
            case TokenKind::Inv:
            case TokenKind::Eq:
            case TokenKind::NotEq:
            case TokenKind::LAngle:
            case TokenKind::RAngle:
            case TokenKind::LE:
            case TokenKind::GE:
            case TokenKind::Spaceship:
            case TokenKind::RefEq:
            case TokenKind::RefNotEq:
            case TokenKind::Range:
            case TokenKind::RangeEQ:
            case TokenKind::Dot: {
                return true;
            }
            default: {
                return false;
            }
        }
    }

    bool Token::isAssignOp() const {
        return utils::arr::has(assignOperators, kind);
    }

    bool Token::isLiteral() const {
        return kind == TokenKind::Lit;
    }

    bool Token::isSomeKeyword() const {
        return kind == TokenKind::Id and asSymbol().isSomeKw();
    }

    bool Token::isPathIdent() const {
        return kind == TokenKind::Id and asSymbol().isPathSeg();
    }

    std::string Token::kindToString(TokenKind kind) {
        const auto found = tokenKindStrings.find(kind);
        if (found != tokenKindStrings.end()) {
            return found->second;
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

    std::string Token::repr(bool prettyQuotes) const {
        std::string str;

        if (prettyQuotes) {
            str += "`";
        }

        switch (kind) {
            case TokenKind::NL: {
                str += "[NL]";
                break;
            }
            case TokenKind::Whitespace: {
                str += "[WS]";
                break;
            }
            case TokenKind::Tab: {
                str += "[TAB]";
                break;
            }
            case TokenKind::LineComment: {
                str += "//" + asSymbol().toString();
                break;
            }
            case TokenKind::BlockComment: {
                str += "/*" + asSymbol().toString() + "*/";
                break;
            }
            case TokenKind::Lit: {
                str += log::fmt(":'", asLit(), "'");
                break;
            }
            case TokenKind::Id: {
                str += asSymbol().toString();
                break;
            }
            default: {
                str += kindToString();
            }
        }

        if (prettyQuotes) {
            return str + "`";
        }

        return str;
    }

    std::string Token::dump(bool withSpan) const {
        std::string str = kindToString();

        switch (kind) {
            case TokenKind::LineComment: {
                str += "//" + asSymbol().toString();
                break;
            }
            case TokenKind::BlockComment: {
                str += "/*" + asSymbol().toString() + "*/";
                break;
            }
            case TokenKind::Lit: {
                str += log::fmt(":'", asLit(), "'");
                break;
            }
            case TokenKind::Id: {
                str += ":'" + asSymbol().toString() + "'";
                break;
            }
            default:;
        }

        if (withSpan) {
            str += " at " + span.toString();
        }

        return str;
    }
}
