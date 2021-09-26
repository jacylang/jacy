#ifndef JACY_SUGGEST_HIGHLIGHTER_H
#define JACY_SUGGEST_HIGHLIGHTER_H

#include "parser/Token.h"
#include "suggest/themes.h"

namespace jc::sugg {
    class Highlighter {
    public:
        Highlighter() = default;
        ~Highlighter() = default;

        std::string highlight(const std::string & source);

        TrueColor getTokColor(const parser::Token & tok, const parser::Token::Opt & nextTok);

    private:
        Theme theme;
    };
}

#endif // JACY_SUGGEST_HIGHLIGHTER_H
