#include "message/Highlighter.h"

namespace jc::message {
    Highlighter::Highlighter() {
        setTheme("ayu-dark");
    }

    std::vector<std::string> Highlighter::builtinTypes = {
        "i8",
        "i16",
        "i32",
        "i64",
        "i128",
        "int",
        "u8",
        "u16",
        "u32",
        "u64",
        "u128",
        "uint",
        "f32",
        "f64",
        "str",
        "char",
        "bool",
        "Option",
        "Result",
        "String",
        "Vec"
    };

    std::vector<TokenKind> Highlighter::opsAsKeywords = {
        TokenKind::Colon,
    };

    std::string Highlighter::highlight(const std::string & source) {
        auto tokens = lexer.lexInternal(source);

        std::stringstream result;

        for (size_t i = 0; i < tokens.size(); i++) {
            const auto & tok = tokens.at(i);
            Token::Opt prevToken = None;
            Token::Opt nextToken = None;
            if (i > 0) {
                prevToken = tokens.at(i - 1);
            }
            if (i < tokens.size() - 1) {
                nextToken = tokens.at(i + 1);
            }
            result << getTokColor(tok, prevToken, nextToken) << tok << log::Color::Reset;
        }

        return result.str();
    }

    TrueColor Highlighter::getTokColor(
        const Token & tok,
        const Token::Opt & prevTok,
        const Token::Opt & nextTok
    ) {
        if (tok.is(TokenKind::Semi)) {
            return theme.semi;
        }

        if (tok.isLiteral()) {
            if (tok.asLit().kind == parser::TokLit::Kind::SQStringLiteral or
                tok.asLit().kind == parser::TokLit::Kind::DQStringLiteral) {
                return theme.string;
            }
            return theme.lit;
        }

        if (tok.is(TokenKind::Id)) {
            if (tok.isSomeKeyword()) {
                return theme.kw;
            }
            const auto & ident = tok.asSymbol().toString();
            if ((ident.at(0) >= 'A' and ident.at(0) <= 'Z') or isBuiltinType(ident)) {
                return theme.type;
            }

            if ((nextTok.some() and nextTok.unwrap().is(TokenKind::LParen))
            or (prevTok.some() and prevTok.unwrap().isKw(span::Kw::Func))) {
                return theme.func;
            }

            return theme.text;
        }

        if (tok.isSomeOp()) {
            return theme.op;
        }

        if (isKeywordOp(tok.kind)) {
            return theme.kw;
        }

        if (tok.is(TokenKind::LineComment) or tok.is(TokenKind::BlockComment)) {
            return theme.comment;
        }

        return theme.text;
    }

    void Highlighter::setTheme(const std::string & themeName) {
        theme = utils::map::expectAt(getThemes(), themeName, "Unknown theme '" + themeName + "'");
    }

    bool Highlighter::isBuiltinType(const std::string & str) const {
        return std::find(builtinTypes.begin(), builtinTypes.end(), str) != builtinTypes.end();
    }

    bool Highlighter::isKeywordOp(TokenKind tokKind) const {
        return std::find(opsAsKeywords.begin(), opsAsKeywords.end(), tokKind) != opsAsKeywords.end();
    }
}
