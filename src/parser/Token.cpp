#include "parser/Token.h"

namespace jc::parser {
    const std::map<std::string, TokenKind> Token::keywords = {
        {"and",         TokenKind::And},
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
        {"import",      TokenKind::Import},
        {"in",          TokenKind::In},
        {"infix",       TokenKind::Infix},
        {"init",        TokenKind::Init},
        {"loop",        TokenKind::Loop},
        {"match",       TokenKind::Match},
        {"mod",         TokenKind::Module},
        {"move",        TokenKind::Move},
        {"mut",         TokenKind::Mut},
        {"not",         TokenKind::Not},
        {"of",          TokenKind::Of},
        {"return",      TokenKind::Return},
        {"or",          TokenKind::Or},
        {"party",       TokenKind::Party},
        {"pub",         TokenKind::Pub},
        {"ref",         TokenKind::Ref},
        {"self",        TokenKind::Self},
        {"static",      TokenKind::Static},
        {"struct",      TokenKind::Struct},
        {"super",       TokenKind::Super},
        {"this",        TokenKind::This},
        {"trait",       TokenKind::Trait},
        {"true",        TokenKind::True},
        {"type",        TokenKind::Type},
        {"use",         TokenKind::Use},
        {"let",         TokenKind::Let},
        {"where",       TokenKind::Where},
        {"while",       TokenKind::While},
    };

    const std::map<TokenKind, std::string> Token::tokenKindStrings = {
        {TokenKind::Eof,             "EOF"},
        {TokenKind::DecLiteral,      "DecLiteral"},
        {TokenKind::BinLiteral,      "BinLiteral"},
        {TokenKind::OctLiteral,      "OctLiteral"},
        {TokenKind::HexLiteral,      "HexLiteral"},
        {TokenKind::FloatLiteral,    "FloatLiteral"},
        {TokenKind::SQStringLiteral, "SQStringLiteral"},
        {TokenKind::Id,              "ID"},

        // Operators //
        {TokenKind::OP,              "OP"},
        {TokenKind::Assign,          "="},
        {TokenKind::Or,              "or"},
        {TokenKind::And,             "and"},

        // Punctuations //
        {TokenKind::Path,            "::"},
        {TokenKind::Spread,          "..."},
        {TokenKind::Pipe,            "|>"},
        {TokenKind::Dollar,          "$"},
        {TokenKind::At,              "@"},
        {TokenKind::Semi,            ";"},
        {TokenKind::Arrow,           "->"},
        {TokenKind::DoubleArrow,     "=>"},
        {TokenKind::LParen,          "("},
        {TokenKind::RParen,          ")"},
        {TokenKind::LBrace,          "{"},
        {TokenKind::RBrace,          "}"},
        {TokenKind::LBracket,        "["},
        {TokenKind::RBracket,        "]"},
        {TokenKind::Comma,           ","},
        {TokenKind::Colon,           ":"},
        {TokenKind::Backtick,        "`"},
        {TokenKind::Wildcard,        "_"},
        {TokenKind::Backslash,       "\\"},
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

    bool Token::isPathIdent() const {
        return kind == TokenKind::Id
            or kind == TokenKind::Super
            or kind == TokenKind::Party
            or kind == TokenKind::Self;
    }

    bool Token::isOp() const {
        switch (kind) {
            case TokenKind::OP:
            case TokenKind::And:
            case TokenKind::Or:
            case TokenKind::Assign:
            case TokenKind::Add:
            case TokenKind::Mul:
            case TokenKind::Ampersand:
            case TokenKind::BitOr:
            case TokenKind::LAngle:
            case TokenKind::RAngle: {
                return true;
            }
            default: {
                return false;
            }
        }
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

    std::string Token::toString(bool prettyQuotes) const {
        std::string str;
        if (prettyQuotes) {
            str += "`";
        }
        switch (kind) {
            case TokenKind::OP:
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
            case TokenKind::OP:
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
