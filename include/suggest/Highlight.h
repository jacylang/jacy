#ifndef JACY_SUGGEST_HIGHLIGHT_H
#define JACY_SUGGEST_HIGHLIGHT_H

#include "log/data_types.h"

namespace jc::sugg {
    using log::TrueColor;

    struct Theme {
        TrueColor comm; // Comment
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

    private:

    };
}

#endif // JACY_SUGGEST_HIGHLIGHT_H
