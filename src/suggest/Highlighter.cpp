#include "suggest/Highlighter.h"

namespace jc::sugg {
    std::string Highlight::highlight(const std::string & source) {
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

    TrueColor Highlight::getTokColor(const parser::Token & tok, const parser::Token::Opt & nextTok) {
        if (tok.isLiteral()) {
            return theme.lit;
        }

        if (tok.is(parser::TokenKind::Id)) {
            if (tok.isSomeKeyword()) {
                return theme.kw;
            }
            const auto & ident = tok.asSymbol().toString();
            if (ident.at(0) >= 'A' and ident.at(0) <= 'Z') {
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
}
