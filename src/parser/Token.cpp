#include "parser/Token.h"

namespace jc::parser {
    const std::map<std::string, TokenType> Token::keywords = {
        {"as",          TokenType::As},
        {"as?",         TokenType::AsQM},
        {"break",       TokenType::Break},
        {"catch",       TokenType::Catch},
        {"class",       TokenType::Class},
        {"const",       TokenType::Const},
        {"continue",    TokenType::Continue},
        {"do",          TokenType::Do},
        {"elif",        TokenType::Elif},
        {"else",        TokenType::Else},
        {"enum",        TokenType::Enum},
        {"export",      TokenType::Export},
        {"false",       TokenType::False},
        {"finally",     TokenType::Finally},
        {"for",         TokenType::For},
        {"func",        TokenType::Func},
        {"get",         TokenType::Get},
        {"if",          TokenType::If},
        {"import",      TokenType::Import},
        {"in",          TokenType::In},
        {"!in",         TokenType::NotIn},
        {"infix",       TokenType::Infix},
        {"init",        TokenType::Init},
        {"is",          TokenType::Is},
        {"!is",         TokenType::NotIs},
        {"loop",        TokenType::Loop},
        {"null",        TokenType::Null},
        {"object",      TokenType::Object},
        {"operator",    TokenType::Operator},
        {"return",      TokenType::Return},
        {"prefix",      TokenType::Prefix},
        {"private",     TokenType::Private},
        {"protected",   TokenType::Protected},
        {"public",      TokenType::Public},
        {"set",         TokenType::Set},
        {"super",       TokenType::Super},
        {"this",        TokenType::This},
        {"throw",       TokenType::Throw},
        {"to",          TokenType::To},
        {"true",        TokenType::True},
        {"try",         TokenType::Try},
        {"type",        TokenType::Type},
        {"val",         TokenType::Val},
        {"var",         TokenType::Var},
        {"when",        TokenType::When},
        {"while",       TokenType::While},
    };

    const std::map<TokenType, std::string> Token::tokenTypeStrings = {
        {TokenType::Eof,                "EOF"},
        {TokenType::Nl,                 "NL"},
        {TokenType::DecLiteral,         "DecLiteral"},
        {TokenType::BinLiteral,         "BinLiteral"},
        {TokenType::OctLiteral,         "OctLiteral"},
        {TokenType::HexLiteral,         "HexLiteral"},
        {TokenType::FloatLiteral,       "FloatLiteral"},
        {TokenType::StringLiteral,      "StringLiteral"},
        {TokenType::Id,                 "ID"},

        // Operators //
        {TokenType::Assign,             "="},
        {TokenType::AddAssign,          "+="},
        {TokenType::SubAssign,          "-="},
        {TokenType::MulAssign,          "*="},
        {TokenType::DivAssign,          "/="},
        {TokenType::ModAssign,          "%="},
        {TokenType::PowerAssign,        "**="},
        {TokenType::ShlAssign,          "<<="},
        {TokenType::ShrAssign,          ">>="},
        {TokenType::BitAndAssign,       "&="},
        {TokenType::BitOrAssign,        "|="},
        {TokenType::XorAssign,          "^="},
        {TokenType::NullishAssign,      "??="},
        {TokenType::Add,                "+"},
        {TokenType::Sub,                "-"},
        {TokenType::Mul,                "*"},
        {TokenType::Div,                "/"},
        {TokenType::Mod,                "%"},
        {TokenType::Power,              "**"},
        {TokenType::Inc,                "++"},
        {TokenType::Dec,                "--"},
        {TokenType::Or,                 "||"},
        {TokenType::And,                "&&"},
        {TokenType::NullCoalesce,       "??"},
        {TokenType::Shl,                "<<"},
        {TokenType::Shr,                ">>"},
        {TokenType::BitAnd,             "&"},
        {TokenType::BitOr,              "|"},
        {TokenType::Xor,                "^"},
        {TokenType::Inv,                "~"},
        {TokenType::Not,                "!"},
        {TokenType::Eq,                 "=="},
        {TokenType::NotEq,              "!="},
        {TokenType::LAngle,             "<"},
        {TokenType::RAngle,             ">"},
        {TokenType::LE,                 "<="},
        {TokenType::GE,                 ">="},
        {TokenType::Spaceship,          "<=>"},
        {TokenType::RefEq,              "==="},
        {TokenType::RefNotEq,           "!=="},
        {TokenType::Range,              ".."},
        {TokenType::RangeLE,            ">.."},
        {TokenType::RangeRE,            "..<"},
        {TokenType::RangeBothE,         ">.<"},
        {TokenType::Dot,                "."},
        {TokenType::SafeCall,           "?."},
        {TokenType::Spread,             "..."},
        {TokenType::Pipe,               "|>"},
        {TokenType::Dollar,             "$"},
        {TokenType::At,                 "@"},
        {TokenType::At_WWS,             "@(WWS)"},

        // Punctuations //
        {TokenType::Semi,               ";"},
        {TokenType::Arrow,              "->"},
        {TokenType::DoubleArrow,        "=>"},
        {TokenType::LParen,             "("},
        {TokenType::RParen,             ")"},
        {TokenType::LBrace,             "{"},
        {TokenType::RBrace,             "}"},
        {TokenType::LBracket,           "["},
        {TokenType::RBracket,           "]"},
        {TokenType::Comma,              ","},
        {TokenType::Colon,              ","},
        {TokenType::Quest,              "?"},
    };

    const std::vector<TokenType> Token::assignOperators = {
        TokenType::Assign,
        TokenType::AddAssign,
        TokenType::SubAssign,
        TokenType::MulAssign,
        TokenType::DivAssign,
        TokenType::ModAssign,
        TokenType::PowerAssign,
        TokenType::ShlAssign,
        TokenType::ShrAssign,
        TokenType::BitAndAssign,
        TokenType::BitOrAssign,
        TokenType::XorAssign,
        TokenType::NullishAssign,
    };

    const std::vector<TokenType> Token::literals = {
        TokenType::DecLiteral,
        TokenType::BinLiteral,
        TokenType::OctLiteral,
        TokenType::HexLiteral,
        TokenType::FloatLiteral,
        TokenType::StringLiteral,
    };

    bool Token::is(TokenType type) const {
        return this->type == type;
    }

    bool Token::isAssignOp() const {
        return utils::arr::has(assignOperators, type);
    }

    bool Token::isLiteral() const {
        return utils::arr::has(literals, type);
    }

    std::string Token::typeToString(const Token & token) {
        const auto found = tokenTypeStrings.find(token.type);
        if (found != tokenTypeStrings.end()) {
            return found->second;
        }

        for (const auto & kw : keywords) {
            if (kw.second == token.type) {
                return kw.first;
            }
        }

        return "No representation";
    }

    std::string Token::typeToString() const {
        return typeToString(*this);
    }

    std::string Token::toString(bool withLoc) const {
        std::string str = typeToString();

        if (type == TokenType::Id or type == TokenType::StringLiteral) {
            str += ":'" + val + "'";
        }

        if (withLoc) {
            str += " at " + loc.toString();
        }

        return str;
    }

    bool Token::isModifier() const {
        return is(TokenType::Infix) or is(TokenType::Operator);
    }
}
