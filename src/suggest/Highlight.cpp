#include "suggest/Highlight.h"

namespace jc::sugg {
    std::string Highlight::highlight(const std::string & source) {
        parser::Lexer lexer;
        auto tokens = lexer.lexInternal(source);

        std::stringstream result;

        for (const auto & tok : tokens) {
            result << getTokColor(tok) << tok << log::Color::Reset;
        }

        return result.str();
    }

    TrueColor Highlight::getTokColor(const parser::Token & tok) {
        if (tok.isLiteral()) {
            return theme.lit;
        }

        if (tok.isSomeKeyword()) {
            return theme.kw;
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
