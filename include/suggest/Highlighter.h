#ifndef JACY_SUGGEST_HIGHLIGHTER_H
#define JACY_SUGGEST_HIGHLIGHTER_H

#include "parser/Token.h"
#include "suggest/themes.h"

namespace jc::sugg {
    class Highlighter {
    public:
        Highlighter();
        ~Highlighter() = default;

        std::string highlight(const std::string & source);

        void setTheme(const std::string & themeName);

    private:
        Theme theme;

        TrueColor getTokColor(const parser::Token & tok, const parser::Token::Opt & nextTok);

        static std::vector<std::string> builtinTypes;
        bool isBuiltinType(const std::string & str) const;
    };
}

#endif // JACY_SUGGEST_HIGHLIGHTER_H
