#include "suggest/Highlighter.h"
#include "parser/Lexer.h"

namespace jc::sugg {
    Highlighter::Highlighter() {
        setTheme("jacy");
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

    std::string Highlighter::highlight(const std::string & source) {
        parser::Lexer lexer;
        auto tokens = lexer.lexInternal(source);

        std::stringstream result;

        for (size_t i = 0; i < tokens.size(); i++) {
            const auto & tok = tokens.at(i);
            parser::Token::Opt nextToken = None;
            if (i < tokens.size() - 1) {
                nextToken = tokens.at(i + 1);
            }
            result << getTokColor(tok, nextToken) << tok << log::Color::Reset;
        }

        return result.str();
    }

    TrueColor Highlighter::getTokColor(const parser::Token & tok, const parser::Token::Opt & nextTok) {
        if (tok.is(parser::TokenKind::Semi)) {
            return theme.semi;
        }

        if (tok.isLiteral()) {
            if (tok.asLit().kind == parser::TokLit::Kind::SQStringLiteral or
                tok.asLit().kind == parser::TokLit::Kind::DQStringLiteral) {
                return theme.string;
            }
            return theme.lit;
        }

        if (tok.is(parser::TokenKind::Id)) {
            if (tok.isSomeKeyword()) {
                return theme.kw;
            }
            const auto & ident = tok.asSymbol().toString();
            if ((ident.at(0) >= 'A' and ident.at(0) <= 'Z') or isBuiltinType(ident)) {
                return theme.type;
            }

            if (nextTok.some() and nextTok.unwrap().is(parser::TokenKind::LParen)) {
                return theme.func;
            }

            return theme.text;
        }

        if (tok.isSomeOp()) {
            return theme.op;
        }

        if (tok.is(parser::TokenKind::LineComment) or tok.is(parser::TokenKind::BlockComment)) {
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
}
