#ifndef JACY_MESSAGE_THEMES_H
#define JACY_MESSAGE_THEMES_H

#include "jon/jon.h"
#include "log/data_types.h"

using namespace jacylang::literal;

namespace jc::message {
    using log::TrueColor;

    static const auto NONE_COLOR = TrueColor {255, 255, 255};

    struct Theme {
        using List = std::vector<Theme>;
        using Map = std::map<std::string, Theme>;

        TrueColor text = NONE_COLOR; // Raw text, e.g. variable
        TrueColor comment = NONE_COLOR; // Line or Block comment
        TrueColor lit = NONE_COLOR; // Literal
        TrueColor kw = NONE_COLOR; // Keyword
        TrueColor op = NONE_COLOR; // Operator
        TrueColor type = NONE_COLOR; // Type color
        TrueColor func = NONE_COLOR; // Function definition/call color
        TrueColor string = NONE_COLOR; // String literal color
        TrueColor semi = NONE_COLOR; // Semicolon `;`
    };

    static inline Theme::Map & getThemes() {
        static auto themeList = R"(
themes: {
    jacy: {
        text: '#d1c2c2'
        comment: '#7a8080'
        literal: '#D696D6'
        keyword: '#E76D83'
        operator: '#E76D83'
        type: '#A2AFEB'
        func: '#81F495'
        string: '#F7D08A'
        semi: '#7a8080'
    }
    dracula: {
        text: '#f8f8f2'
        comment: '#6272a4'
        literal: '#bd93f9'
        keyword: '#ff79c6'
        operator: '#ff79c6'
        type: '#8be9fd'
        func: '#50fa7b'
        string: '#f1fa8c'
        semi: '#6272a4'
    }
    ayu-dark: {
        text: '#cccac2'
        comment: '#b8cfe6'
        literal: '#dfbfff'
        keyword: '#ffad66'
        operator: '#f29e74'
        type: '#73d0ff'
        func: '#ffd173'
        string: '#d5ff80'
        semi: '#ACB6BF'
    }
}
)"_jon;

        static bool inited = false;
        static Theme::Map themes;

        if (not inited) {
            for (const auto & th : themeList.objAt("themes")) {
                const auto & themeName = th.first;
                Theme theme;

                for (const auto & c : th.second.getObj()) {
                    const auto & rule = c.first;
                    const auto & color = c.second.getStr();
                    if (rule == "text") {
                        theme.text = color;
                    } else if (rule == "comment") {
                        theme.comment = color;
                    } else if (rule == "literal") {
                        theme.lit = color;
                    } else if (rule == "keyword") {
                        theme.kw = color;
                    } else if (rule == "operator") {
                        theme.op = color;
                    } else if (rule == "type") {
                        theme.type = color;
                    } else if (rule == "func") {
                        theme.func = color;
                    } else if (rule == "string") {
                        theme.string = color;
                    } else if (rule == "semi") {
                        theme.semi = color;
                    } else {
                        throw std::logic_error("Unknown theme entity '" + rule + "' in theme" + themeName);
                    }
                }

                themes.emplace(themeName, theme);
            }
        }

        return themes;
    }
}

#endif // JACY_MESSAGE_THEMES_H
