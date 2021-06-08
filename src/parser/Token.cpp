#include "parser/Token.h"

namespace jc::parser {
    const std::map<std::string, TokenKind> Token::keywords = {
        {"as",          TokenKind::As},
        {"async",       TokenKind::Async},
        {"await",       TokenKind::Await},
        {"break",       TokenKind::Break},
        {"const",       TokenKind::Const},
        {"continue",    TokenKind::Continue},
        {"do",          TokenKind::Do},
        {"elif",        TokenKind::Elif},
        {"else",        TokenKind::Else},
        {"enum",        TokenKind::Enum},
        {"false",       TokenKind::False},
        {"for",         TokenKind::For},
        {"func",        TokenKind::Func},
        {"if",          TokenKind::If},
        {"impl",        TokenKind::Impl},
        {"in",          TokenKind::In},
        {"!in",         TokenKind::NotIn},
        {"infix",       TokenKind::Infix},
        {"init",        TokenKind::Init},
        {"loop",        TokenKind::Loop},
        {"mod",         TokenKind::Module},
        {"move",        TokenKind::Move},
        {"mut",         TokenKind::Mut},
        {"return",      TokenKind::Return},
        {"party",       TokenKind::Party},
        {"pri",         TokenKind::Pri},
        {"pub",         TokenKind::Pub},
        {"self",        TokenKind::Self},
        {"static",      TokenKind::Static},
        {"struct",      TokenKind::Struct},
        {"super",       TokenKind::Super},
        {"this",        TokenKind::This},
        {"trait",       TokenKind::Trait},
        {"true",        TokenKind::True},
        {"type",        TokenKind::Type},
        {"union",       TokenKind::Union},
        {"unsafe",      TokenKind::Unsafe},
        {"use",         TokenKind::Use},
        {"val",         TokenKind::Val},
        {"var",         TokenKind::Var},
        {"when",        TokenKind::When},
        {"where",       TokenKind::Where},
        {"while",       TokenKind::While},
        {"yield",       TokenKind::Yield},
    };

    const std::map<TokenKind, std::string> Token::tokenKindStrings = {
        {TokenKind::Eof,                "EOF"},
        {TokenKind::Nl,                 "NL"},
        {TokenKind::DecLiteral,         "DecLiteral"},
        {TokenKind::BinLiteral,         "BinLiteral"},
        {TokenKind::OctLiteral,         "OctLiteral"},
        {TokenKind::HexLiteral,         "HexLiteral"},
        {TokenKind::FloatLiteral,       "FloatLiteral"},
        {TokenKind::SQStringLiteral,    "SQStringLiteral"},
        {TokenKind::Id,                 "ID"},

        // Operators //
        {TokenKind::Assign,             "="},
        {TokenKind::AddAssign,          "+="},
        {TokenKind::SubAssign,          "-="},
        {TokenKind::MulAssign,          "*="},
        {TokenKind::DivAssign,          "/="},
        {TokenKind::ModAssign,          "%="},
        {TokenKind::PowerAssign,        "**="},
        {TokenKind::ShlAssign,          "<<="},
        {TokenKind::ShrAssign,          ">>="},
        {TokenKind::BitAndAssign,       "&="},
        {TokenKind::BitOrAssign,        "|="},
        {TokenKind::XorAssign,          "^="},
        {TokenKind::NullishAssign,      "??="},
        {TokenKind::Add,                "+"},
        {TokenKind::Sub,                "-"},
        {TokenKind::Mul,                "*"},
        {TokenKind::Div,                "/"},
        {TokenKind::Mod,                "%"},
        {TokenKind::Power,              "**"},
        {TokenKind::Or,                 "||"},
        {TokenKind::And,                "&&"},
        {TokenKind::NullCoalesce,       "??"},
        {TokenKind::Shl,                "<<"},
        {TokenKind::Shr,                ">>"},
        {TokenKind::BitAnd,             "&"},
        {TokenKind::BitOr,              "|"},
        {TokenKind::Xor,                "^"},
        {TokenKind::Inv,                "~"},
        {TokenKind::Not,                "!"},
        {TokenKind::Eq,                 "=="},
        {TokenKind::NotEq,              "!="},
        {TokenKind::LAngle,             "<"},
        {TokenKind::RAngle,             ">"},
        {TokenKind::LE,                 "<="},
        {TokenKind::GE,                 ">="},
        {TokenKind::Spaceship,          "<=>"},
        {TokenKind::RefEq,              "==="},
        {TokenKind::RefNotEq,           "!=="},
        {TokenKind::Range,              ".."},
        {TokenKind::RangeEQ,            "..="},
        {TokenKind::Dot,                "."},
        {TokenKind::Path,               "::"},
        {TokenKind::Spread,             "..."},
        {TokenKind::Pipe,               "|>"},
        {TokenKind::Dollar,             "$"},
        {TokenKind::At,                 "@"},
        {TokenKind::At_WWS,             "@(WWS)"},

        // Punctuations //
        {TokenKind::Semi,               ";"},
        {TokenKind::Arrow,              "->"},
        {TokenKind::DoubleArrow,        "=>"},
        {TokenKind::LParen,             "("},
        {TokenKind::RParen,             ")"},
        {TokenKind::LBrace,             "{"},
        {TokenKind::RBrace,             "}"},
        {TokenKind::LBracket,           "["},
        {TokenKind::RBracket,           "]"},
        {TokenKind::Comma,              ","},
        {TokenKind::Colon,              ":"},
        {TokenKind::Quest,              "?"},
        {TokenKind::Backtick,           "`"},
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
        TokenKind::NullishAssign,
    };

    const std::vector<TokenKind> Token::literals = {
        TokenKind::DecLiteral,
        TokenKind::BinLiteral,
        TokenKind::OctLiteral,
        TokenKind::HexLiteral,
        TokenKind::FloatLiteral,
        TokenKind::SQStringLiteral,
        TokenKind::DQStringLiteral,
    };

    bool Token::is(TokenKind kind) const {
        return this->kind == kind;
    }

    bool Token::isAssignOp() const {
        return utils::arr::has(assignOperators, kind);
    }

    bool Token::isLiteral() const {
        return utils::arr::has(literals, kind);
    }

    bool Token::isKw() const {
        for (const auto & kw : keywords) {
            if (kw.second == kind) {
                return true;
            }
        }
        return false;
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

    std::string Token::toString(bool prettyQuotes) const {
        std::string str;
        if (prettyQuotes) {
            str += "`";
        }
        switch (kind) {
            case TokenKind::DecLiteral:
            case TokenKind::BinLiteral:
            case TokenKind::OctLiteral:
            case TokenKind::HexLiteral:
            case TokenKind::FloatLiteral:
            case TokenKind::SQStringLiteral:
            case TokenKind::DQStringLiteral:
            case TokenKind::Id: {
                str += val;
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
            case TokenKind::DecLiteral:
            case TokenKind::BinLiteral:
            case TokenKind::OctLiteral:
            case TokenKind::HexLiteral:
            case TokenKind::FloatLiteral:
            case TokenKind::SQStringLiteral:
            case TokenKind::DQStringLiteral:
            case TokenKind::Id: {
                str += ":'" + val + "'";
            }
            default:;
        }

        if (withSpan) {
            str += " at " + span.toString();
        }

        return str;
    }
}
