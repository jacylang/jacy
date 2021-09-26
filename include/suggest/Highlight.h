#ifndef JACY_SUGGEST_HIGHLIGHT_H
#define JACY_SUGGEST_HIGHLIGHT_H

#include "log/data_types.h"
#include "parser/Lexer.h"

namespace jc::sugg {
    using log::TrueColor;

    struct Theme {
        TrueColor text; // Raw text, e.g. variable
        TrueColor comment; // Line or Block comment
        TrueColor lit; // Literal
        TrueColor kw; // Keyword
        TrueColor op; // Operator
        TrueColor type; // Type color
    };

    class Highlight {
    public:
        Highlight() = default;
        ~Highlight() = default;

        std::string highlight(const std::string & source);

        TrueColor getTokColor(const parser::Token & tok);

    private:
        Theme theme;
    };
}

#endif // JACY_SUGGEST_HIGHLIGHT_H
