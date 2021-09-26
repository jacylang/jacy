#ifndef JACY_SUGGEST_THEMES_H
#define JACY_SUGGEST_THEMES_H

#include "jon/jon.h"
#include "log/data_types.h"

using namespace jacylang::literal;

namespace jc::sugg {
    using log::TrueColor;

    struct Theme {
        using List = std::vector<Theme>;
        using Map = std::map<std::string, Theme>;

        TrueColor text{248, 248, 242}; // Raw text, e.g. variable
        TrueColor comment{98, 114, 164}; // Line or Block comment
        TrueColor lit{189, 147, 249}; // Literal
        TrueColor kw{255, 121, 198}; // Keyword
        TrueColor op{255, 121, 198}; // Operator
        TrueColor type{139, 233, 253}; // Type color
        TrueColor func{80, 250, 123}; // Function definition/call color
        TrueColor string{241, 250, 140}; // String literal color
    };

    static inline Theme::Map & getThemes() {
        static auto themes = R"(
themes: {
    dracula: {

    }
}
        )"_jon;

    }
}

#endif // JACY_SUGGEST_THEMES_H
