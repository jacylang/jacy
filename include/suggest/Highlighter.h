#ifndef JACY_SUGGEST_HIGHLIGHTER_H
#define JACY_SUGGEST_HIGHLIGHTER_H

#include "log/data_types.h"
#include "parser/Lexer.h"

namespace jc::sugg {
    using log::TrueColor;

    struct Theme {
        TrueColor text{248, 248, 242}; // Raw text, e.g. variable
        TrueColor comment{98, 114, 164}; // Line or Block comment
        TrueColor lit{189, 147, 249}; // Literal
        TrueColor kw{255, 121, 198}; // Keyword
        TrueColor op{255, 121, 198}; // Operator
        TrueColor type{139, 233, 253}; // Type color
        TrueColor func{80, 250, 123}; // Type color
    };

    class Highlight {
    public:
        Highlight() = default;
        ~Highlight() = default;

        std::string highlight(const std::string & source);

        TrueColor getTokColor(const parser::Token & tok, const parser::Token::Opt & nextTok);

    private:
        Theme theme;
    };
}

#endif // JACY_SUGGEST_HIGHLIGHTER_H
